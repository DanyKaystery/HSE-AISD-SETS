#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

class ArrayGenerator {
private:
    std::mt19937 rng;
    static constexpr int MIN_VALUE = 0;
    static constexpr int MAX_VALUE = 10000;
    static constexpr int MAX_ARRAY_SIZE = 100000;

public:
    explicit ArrayGenerator(long seed = std::random_device{}()) : rng(seed) {}

    std::vector<int> generateRandom(int size) {
        std::uniform_int_distribution<int> dist(MIN_VALUE, MAX_VALUE);
        std::vector<int> v(MAX_ARRAY_SIZE);
        for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
            v[i] = dist(rng);
        }
        return {v.begin(), v.begin() + size};
    }

    std::vector<int> generateReverse(int size) {
        std::uniform_int_distribution dist(MIN_VALUE, MAX_VALUE);
        std::vector<int> v(MAX_ARRAY_SIZE);
        for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
            v[i] = dist(rng);
        }
        std::sort(v.begin(), v.end(), std::greater());
        return {v.begin(), v.begin() + size};
    }

    std::vector<int> generateNearly(int size, double sp = 5.0) {
        std::uniform_int_distribution dist(MIN_VALUE, MAX_VALUE);
        std::vector<int> v(MAX_ARRAY_SIZE);
        for (int i = 0; i < MAX_ARRAY_SIZE; ++i) {
            v[i] = dist(rng);
        }
        std::sort(v.begin(), v.end());
        int sc = static_cast<int>(size * sp / 100.0);
        std::uniform_int_distribution<int> idxDist(0, size - 1);
        for (int i = 0; i < sc; ++i) {
            int idx1 = idxDist(rng);
            int idx2 = idxDist(rng);
            std::swap(v[idx1], v[idx2]);
        }
        return {v.begin(), v.begin() + size};
    }

    struct TestArrays {
        std::vector<int> random;
        std::vector<int> reverseSorted;
        std::vector<int> nearlySorted;
    };

    TestArrays generateFullTestSet() {
        return {
            generateRandom(MAX_ARRAY_SIZE),
            generateReverse(MAX_ARRAY_SIZE),
            generateNearly(MAX_ARRAY_SIZE, 5.0)
        };
    }

    static std::vector<int> getSubarray(const std::vector<int>& v, int size) {
        return {v.begin(), v.begin() + size};
    }

    static constexpr int MIN_SIZE = 500;
    static constexpr int MAX_SIZE = 100000;
    static constexpr int STEP = 100;
};

void insertionSort(std::vector<int>& v, int left, int right) {
    for (int i = left + 1; i <= right; i++) {
        int k = v[i];
        int j = i - 1;
        while (j >= left && k < v[j]) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = k;
    }
}

void merge(std::vector<int>& v, int l, int mid, int r) {
    int lenl = mid - l + 1;
    int lenr = r - mid;

    std::vector<int> left(lenl);
    std::vector<int> right(lenr);

    for (int i = 0; i < lenl; i++)
        left[i] = v[l + i];
    for (int j = 0; j < lenr; j++)
        right[j] = v[mid + 1 + j];

    int k = l;
    int i = 0;
    int j = 0;
    while (i < lenl && j < lenr) {
        if (left[i] <= right[j]) {
            v[k] = left[i];
            i++;
        } else {
            v[k] = right[j];
            j++;
        }
        k++;
    }

    while (i < lenl) {
        v[k] = left[i];
        i++;
        k++;
    }
    while (j < lenr) {
        v[k] = right[j];
        j++;
        k++;
    }
}

void mergeSortStandard(std::vector<int>& v, int left, int right) {
    if (left >= right)
        return;

    int mid = left + (right - left) / 2;
    mergeSortStandard(v, left, mid);
    mergeSortStandard(v, mid + 1, right);
    merge(v, left, mid, right);
}

void mergeSortHybrid(std::vector<int>& v, int left, int right, int minIdxStartInsertion) {
    if (left >= right)
        return;

    if (right - left + 1 <= minIdxStartInsertion) {
        insertionSort(v, left, right);
        return;
    }

    int mid = left + (right - left) / 2;
    mergeSortHybrid(v, left, mid, minIdxStartInsertion);
    mergeSortHybrid(v, mid + 1, right, minIdxStartInsertion);
    merge(v, left, mid, right);
}

class SortTester {
public:
    static long long testStandardMergeSort(std::vector<int> v) {
        auto start = std::chrono::high_resolution_clock::now();
        mergeSortStandard(v, 0, static_cast<int>(v.size() - 1));
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    }

    static long long testHybridMergeSort(std::vector<int> v, int threshold) {
        auto start = std::chrono::high_resolution_clock::now();
        mergeSortHybrid(v, 0, static_cast<int>(v.size() - 1), threshold);
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    ArrayGenerator gen(42);
    auto testSets = gen.generateFullTestSet();

    int minIdxStartInsertion = 30;

    for (int size = ArrayGenerator::MIN_SIZE; size <= ArrayGenerator::MAX_SIZE; size += ArrayGenerator::STEP) {
        long long standardRandom = 0;
        long long hybridRandom = 0;
        long long standardReverse = 0;
        long long hybridReverse = 0;
        long long standardNearly = 0;
        long long hybridNearly = 0;
        for (int i = 0; i < 5; i++) {
            auto randomArr = ArrayGenerator::getSubarray(testSets.random, size);
            auto reverseArr = ArrayGenerator::getSubarray(testSets.reverseSorted, size);
            auto nearlySortedArr = ArrayGenerator::getSubarray(testSets.nearlySorted, size);

            standardRandom += SortTester::testStandardMergeSort(randomArr);
            hybridRandom += SortTester::testHybridMergeSort(randomArr, minIdxStartInsertion);
            standardReverse += SortTester::testStandardMergeSort(reverseArr);
            hybridReverse += SortTester::testHybridMergeSort(reverseArr, minIdxStartInsertion);
            standardNearly += SortTester::testStandardMergeSort(nearlySortedArr);
            hybridNearly += SortTester::testHybridMergeSort(nearlySortedArr, minIdxStartInsertion);
        }


        std::cout << size << " "
                  << standardRandom/5 << " "
                  << hybridRandom/5 << " "
                  << standardReverse/5 << " "
                  << hybridReverse/5 << " "
                  << standardNearly/5 << " "
                  << hybridNearly/5 << std::endl;
        std::cout.flush();
    }

    return 0;
}