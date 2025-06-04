#ifndef CUSTUM_VECTOR_H
#define CUSTUM_VECTOR_H

#include <cstddef>      // for size_t
#include <stdexcept>    // for std::out_of_range
#include <utility>      // for std::move, std::forward
#include <memory>       // for std::allocator, std::allocator_traits

template <typename T, typename Allocator = std::allocator<T>>
class cmsVector {

private:
    T* m_data;
    size_t m_size;
    size_t m_capacity;
    Allocator m_allocator;

    // 재할당 유틸리티 함수
    void reallocate(size_t new_capacity) {
        T* new_data = m_allocator.allocate(new_capacity);

        // 기존 요소 이동
        for (size_t i = 0; i < m_size; ++i) {
            new (new_data + i) T(std::move(m_data[i]));
            m_data[i].~T();
        }

        // 기존 메모리 해제
        if (m_data) {
            m_allocator.deallocate(m_data, m_capacity);
        }

        m_data = new_data;
        m_capacity = new_capacity;
    }
public:
    // 기본 생성자
    cmsVector() : m_data(nullptr), m_size(0), m_capacity(0) {}

    // 크기 지정 생성자
    explicit cmsVector(size_t size) : m_size(size), m_capacity(size) {
        if (size > 0) {
            m_data = m_allocator.allocate(size);
            for (size_t i = 0; i < size; ++i) {
                new (m_data + i) T();
            }
        }
        else {
            m_data = nullptr;
        }
    }

    // 크기와 초기값 지정 생성자
    cmsVector(size_t size, const T& value) : m_size(size), m_capacity(size) {
        if (size > 0) {
            m_data = m_allocator.allocate(size);
            for (size_t i = 0; i < size; ++i) {
                new (m_data + i) T(value);
            }
        }
        else {
            m_data = nullptr;
        }
    }

    // 초기화 리스트 생성자
    cmsVector(std::initializer_list<T> init) :
        m_size(init.size()), m_capacity(init.size()) {
        if (m_size > 0) {
            m_data = m_allocator.allocate(m_size);
            size_t i = 0;
            for (const auto& item : init) {
                new (m_data + i) T(item);
                ++i;
            }
        }
        else {
            m_data = nullptr;
        }
    }

    // 복사 생성자
    cmsVector(const cmsVector& other) :
        m_size(other.m_size), m_capacity(other.m_size) {
        if (m_size > 0) {
            m_data = m_allocator.allocate(m_size);
            for (size_t i = 0; i < m_size; ++i) {
                new (m_data + i) T(other.m_data[i]);
            }
        }
        else {
            m_data = nullptr;
        }
    }

    // 이동 생성자
    cmsVector(cmsVector&& other) noexcept :
        m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // 소멸자
    ~cmsVector() {
        clear();
        if (m_data) {
            m_allocator.deallocate(m_data, m_capacity);
        }
    }

    // 복사 대입 연산자
    cmsVector& operator=(const cmsVector& other) {
        if (this != &other) {
            clear();
            if (m_capacity < other.m_size) {
                if (m_data) {
                    m_allocator.deallocate(m_data, m_capacity);
                }
                m_capacity = other.m_size;
                m_data = m_allocator.allocate(m_capacity);
            }

            m_size = other.m_size;
            for (size_t i = 0; i < m_size; ++i) {
                new (m_data + i) T(other.m_data[i]);
            }
        }
        return *this;
    }

    // 이동 대입 연산자
    cmsVector& operator=(cmsVector&& other) noexcept {
        if (this != &other) {
            clear();
            if (m_data) {
                m_allocator.deallocate(m_data, m_capacity);
            }

            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;

            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // 요소 접근 연산자
    T& operator[](size_t index) {
        return m_data[index];
    }

    const T& operator[](size_t index) const {
        return m_data[index];
    }

    // 안전한 요소 접근
    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        return m_data[index];
    }

    // 첫 번째 요소 반환
    T& front() {
        return m_data[0];
    }

    const T& front() const {
        return m_data[0];
    }

    // 마지막 요소 반환
    T& back() {
        return m_data[m_size - 1];
    }

    const T& back() const {
        return m_data[m_size - 1];
    }

    // 원시 포인터 반환
    T* data() {
        return m_data;
    }

    const T* data() const {
        return m_data;
    }

    // 이터레이터 지원
    T* begin() {
        return m_data;
    }

    const T* begin() const {
        return m_data;
    }

    T* end() {
        return m_data + m_size;
    }

    const T* end() const {
        return m_data + m_size;
    }

    // 크기 관련 함수들
    cBool empty() const {
        return m_size == 0;
    }

    size_t size() const {
        return m_size;
    }

    size_t capacity() const {
        return m_capacity;
    }

    // 용량 예약
    void reserve(size_t new_capacity) {
        if (new_capacity > m_capacity) {
            reallocate(new_capacity);
        }
    }

    // 크기 조정
    void resize(size_t new_size) {
        if (new_size > m_capacity) {
            reallocate(new_size);
        }

        // 새 요소 초기화
        if (new_size > m_size) {
            for (size_t i = m_size; i < new_size; ++i) {
                new (m_data + i) T();
            }
        }
        else if (new_size < m_size) {
            // 초과 요소 제거
            for (size_t i = new_size; i < m_size; ++i) {
                m_data[i].~T();
            }
        }

        m_size = new_size;
    }

    // 크기 조정 (초기값 지정)
    void resize(size_t new_size, const T& value) {
        if (new_size > m_capacity) {
            reallocate(new_size);
        }

        // 새 요소 초기화
        if (new_size > m_size) {
            for (size_t i = m_size; i < new_size; ++i) {
                new (m_data + i) T(value);
            }
        }
        else if (new_size < m_size) {
            // 초과 요소 제거
            for (size_t i = new_size; i < m_size; ++i) {
                m_data[i].~T();
            }
        }

        m_size = new_size;
    }

    // 용량 줄이기
    void shrink_to_fit() {
        if (m_size < m_capacity) {
            reallocate(m_size);
        }
    }

    // 요소 추가
    void push_back(const T& value) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(value);
        ++m_size;
    }

    // 이동 의미론을 사용한 요소 추가
    void push_back(T&& value) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(std::move(value));
        ++m_size;
    }

    // 요소 생성 추가
    template<typename... Args>
    T& emplace_back(Args&&... args) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(std::forward<Args>(args)...);
        return m_data[m_size++];
    }

    // 마지막 요소 제거
    void pop_back() {
        if (m_size > 0) {
            --m_size;
            m_data[m_size].~T();
        }
    }

    // 모든 요소 제거
    void clear() {
        for (size_t i = 0; i < m_size; ++i) {
            m_data[i].~T();
        }
        m_size = 0;
    }
};


#endif // !CUSTUM_VECTOR_H
