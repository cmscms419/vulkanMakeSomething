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

    // ���Ҵ� ��ƿ��Ƽ �Լ�
    void reallocate(size_t new_capacity) {
        T* new_data = m_allocator.allocate(new_capacity);

        // ���� ��� �̵�
        for (size_t i = 0; i < m_size; ++i) {
            new (new_data + i) T(std::move(m_data[i]));
            m_data[i].~T();
        }

        // ���� �޸� ����
        if (m_data) {
            m_allocator.deallocate(m_data, m_capacity);
        }

        m_data = new_data;
        m_capacity = new_capacity;
    }
public:
    // �⺻ ������
    cmsVector() : m_data(nullptr), m_size(0), m_capacity(0) {}

    // ũ�� ���� ������
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

    // ũ��� �ʱⰪ ���� ������
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

    // �ʱ�ȭ ����Ʈ ������
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

    // ���� ������
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

    // �̵� ������
    cmsVector(cmsVector&& other) noexcept :
        m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity) {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // �Ҹ���
    ~cmsVector() {
        clear();
        if (m_data) {
            m_allocator.deallocate(m_data, m_capacity);
        }
    }

    // ���� ���� ������
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

    // �̵� ���� ������
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

    // ��� ���� ������
    T& operator[](size_t index) {
        return m_data[index];
    }

    const T& operator[](size_t index) const {
        return m_data[index];
    }

    // ������ ��� ����
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

    // ù ��° ��� ��ȯ
    T& front() {
        return m_data[0];
    }

    const T& front() const {
        return m_data[0];
    }

    // ������ ��� ��ȯ
    T& back() {
        return m_data[m_size - 1];
    }

    const T& back() const {
        return m_data[m_size - 1];
    }

    // ���� ������ ��ȯ
    T* data() {
        return m_data;
    }

    const T* data() const {
        return m_data;
    }

    // ���ͷ����� ����
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

    // ũ�� ���� �Լ���
    cBool empty() const {
        return m_size == 0;
    }

    size_t size() const {
        return m_size;
    }

    size_t capacity() const {
        return m_capacity;
    }

    // �뷮 ����
    void reserve(size_t new_capacity) {
        if (new_capacity > m_capacity) {
            reallocate(new_capacity);
        }
    }

    // ũ�� ����
    void resize(size_t new_size) {
        if (new_size > m_capacity) {
            reallocate(new_size);
        }

        // �� ��� �ʱ�ȭ
        if (new_size > m_size) {
            for (size_t i = m_size; i < new_size; ++i) {
                new (m_data + i) T();
            }
        }
        else if (new_size < m_size) {
            // �ʰ� ��� ����
            for (size_t i = new_size; i < m_size; ++i) {
                m_data[i].~T();
            }
        }

        m_size = new_size;
    }

    // ũ�� ���� (�ʱⰪ ����)
    void resize(size_t new_size, const T& value) {
        if (new_size > m_capacity) {
            reallocate(new_size);
        }

        // �� ��� �ʱ�ȭ
        if (new_size > m_size) {
            for (size_t i = m_size; i < new_size; ++i) {
                new (m_data + i) T(value);
            }
        }
        else if (new_size < m_size) {
            // �ʰ� ��� ����
            for (size_t i = new_size; i < m_size; ++i) {
                m_data[i].~T();
            }
        }

        m_size = new_size;
    }

    // �뷮 ���̱�
    void shrink_to_fit() {
        if (m_size < m_capacity) {
            reallocate(m_size);
        }
    }

    // ��� �߰�
    void push_back(const T& value) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(value);
        ++m_size;
    }

    // �̵� �ǹ̷��� ����� ��� �߰�
    void push_back(T&& value) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(std::move(value));
        ++m_size;
    }

    // ��� ���� �߰�
    template<typename... Args>
    T& emplace_back(Args&&... args) {
        if (m_size == m_capacity) {
            size_t new_capacity = m_capacity == 0 ? 1 : m_capacity * 2;
            reallocate(new_capacity);
        }

        new (m_data + m_size) T(std::forward<Args>(args)...);
        return m_data[m_size++];
    }

    // ������ ��� ����
    void pop_back() {
        if (m_size > 0) {
            --m_size;
            m_data[m_size].~T();
        }
    }

    // ��� ��� ����
    void clear() {
        for (size_t i = 0; i < m_size; ++i) {
            m_data[i].~T();
        }
        m_size = 0;
    }
};


#endif // !CUSTUM_VECTOR_H
