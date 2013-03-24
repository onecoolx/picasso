/* Picasso - a vector graphics library
 * 
 * Copyright (C) 2011 Zhang Ji Peng
 * Contact: onecoolx@gmail.com
 */

#ifndef _DATA_VECTOR_H_
#define _DATA_VECTOR_H_

#include "common.h"
#include <string.h>

namespace picasso {

template<typename T> struct pod_allocator
{
    static T*   allocate(unsigned int num)       { return new T [num]; }
    static void deallocate(T* ptr, unsigned int) { delete [] ptr;      }
};

//------------------------------------------------------------------------
template<typename T> class pod_vector
{
public:
    pod_vector() 
		: m_size(0)
		, m_capacity(0)
		, m_array(0) 
	{
	}

    ~pod_vector() 
	{ 
		pod_allocator<T>::deallocate(m_array, m_capacity);
   	}

    pod_vector(unsigned int cap);

    // Copying
    pod_vector(const pod_vector<T>&);

    const pod_vector<T>& operator = (const pod_vector<T>&);

    // Set new capacity. All data is lost, size is set to zero.
    void capacity(unsigned int cap);
    unsigned int capacity(void) const { return m_capacity; }
    unsigned int size(void) const { return m_size; }

    // Resize keeping the content.
    void resize(unsigned int new_size);

    bool push_back(const T& v);   

	bool is_full(void) const { return m_size == m_capacity;}

    bool insert_at(unsigned int pos, const T& val);

    bool set_data(unsigned int num, T* data);

    const T& operator [] (unsigned int i) const { return m_array[i]; }
          T& operator [] (unsigned int i)       { return m_array[i]; }

    const T* data(void) const { return m_array; }
          T* data(void)       { return m_array; }

    void clear(void)              { m_size = 0; }
    void remove_last(void) { if (m_size) --m_size; }
    void cut_at(unsigned int num) { if (num < m_size) m_size = num; }

protected:
    unsigned m_size;
    unsigned m_capacity;
    T*       m_array;
};

//------------------------------------------------------------------------
template<typename T> 
void pod_vector<T>::capacity(unsigned int cap)
{
    m_size = 0;
    if (cap > m_capacity) {
        pod_allocator<T>::deallocate(m_array, m_capacity);
        m_capacity = cap;
        m_array = m_capacity ? pod_allocator<T>::allocate(m_capacity) : 0;
    }
}

//------------------------------------------------------------------------
template<typename T> 
void pod_vector<T>::resize(unsigned new_size)
{
    if (new_size > m_size) {
        if (new_size > m_capacity) {
            T* data = pod_allocator<T>::allocate(new_size);
            memcpy(data, m_array, m_size * sizeof(T));
            pod_allocator<T>::deallocate(m_array, m_capacity);
			m_capacity = new_size;
            m_array = data;
        }
    } else {
        m_size = new_size;
    }
}

//------------------------------------------------------------------------
template<typename T> pod_vector<T>::pod_vector(unsigned int cap)
   	: m_size(0)
	, m_capacity(cap)
	, m_array(pod_allocator<T>::allocate(m_capacity)) 
{
}

//------------------------------------------------------------------------
template<typename T> pod_vector<T>::pod_vector(const pod_vector<T>& v) :
    m_size(v.m_size),
    m_capacity(v.m_capacity),
    m_array(v.m_capacity ? pod_allocator<T>::allocate(v.m_capacity) : 0)
{
    memcpy(m_array, v.m_array, sizeof(T) * v.m_size);
}

//------------------------------------------------------------------------
template<typename T> 
const pod_vector<T>& pod_vector<T>::operator = (const pod_vector<T>&v)
{
	if (this == &v)
		return *this;

    pod_allocator<T>::deallocate(m_array, m_capacity);
	m_capacity = v.m_capacity;
	m_size = v.m_size;
	m_array = 0;

	if (m_capacity)
		m_array = pod_allocator<T>::allocate(m_capacity);

    if (m_size) 
		memcpy(m_array, v.m_array, sizeof(T) * v.m_size);

    return *this;
}

//------------------------------------------------------------------------
template<typename T> 
bool pod_vector<T>::push_back(const T& v)   
{
	if (m_size >= m_capacity)
		return false;

	m_array[m_size++] = v;
	return true;
}

//------------------------------------------------------------------------
template<typename T> 
bool pod_vector<T>::insert_at(unsigned int pos, const T& val)
{
	if (pos >= m_capacity)
		return false;

    if (pos >= m_size) {
        m_array[m_size] = val;
    } else {
        memmove(m_array + pos + 1, m_array + pos, (m_size - pos) * sizeof(T));
        m_array[pos] = val;
    }
    ++m_size;
	return true;
}

//------------------------------------------------------------------------
template<typename T> 
bool pod_vector<T>::set_data(unsigned int num, T* data)
{
    if (!num || !data)
        return false;

    if (num > m_capacity)
        return false;

    m_size = num;
    memcpy(m_array, data, sizeof(T) * m_size);
    return true;
}


// pod_bvector
template <typename T> class pod_bvector : public pod_vector<T>
{
public:
	typedef pod_vector<T> base_type;

	enum {
		block_size = 4,
	};

	void add(const T& v)
	{
		if (!base_type::capacity()) {
			base_type::resize(block_size);
		}

		if (base_type::is_full()) {
			base_type::resize(base_type::capacity() << 1);
		}

		base_type::push_back(v);
	}

	void remove_all(void)
	{
		pod_allocator<T>::deallocate(base_type::m_array, base_type::m_capacity);
		base_type::m_array = 0;
		base_type::m_capacity = 0;
		base_type::m_size = 0;
	}

private:
	bool push_back(const T& v);
    bool insert_at(unsigned int pos, const T& val);
};

// block_allocator
class block_allocator 
{
    typedef struct {
        byte*        data;
        unsigned int size;
    } block_type;

public:

    block_allocator(unsigned block_size, unsigned block_ptr_inc = 256-8) 
        : m_block_size(block_size)
        , m_block_ptr_inc(block_ptr_inc)
        , m_num_blocks(0)
        , m_max_blocks(0)
        , m_blocks(0)
        , m_buf_ptr(0)
        , m_remain_size(0)
    {
#if _DEBUG
        m_all_mem = 0;
#endif
    }

    ~block_allocator()
    {
        remove_all();
    }

    byte* allocate(unsigned int size, unsigned int alignment = 1)
    {
        if (!size) 
            return 0;

        if (size <= m_remain_size) {
            byte* ptr = m_buf_ptr;
            if (alignment > 1) {
                unsigned int align = (alignment - ((unsigned int)(ptr)) % alignment) % alignment;

                size += align;
                ptr += align;

                if (size <= m_remain_size) {
                    m_remain_size -= size;
                    m_buf_ptr += size;
                    return ptr;
                }

                allocate_block(size);
                return allocate(size - align, alignment);
            }

            m_remain_size -= size;
            m_buf_ptr += size;
            return ptr;
        }
        allocate_block(size + alignment - 1);
        return allocate(size, alignment);
    }

    void remove_all(void)
    {
        if (m_num_blocks) {
            block_type* blk = m_blocks + m_num_blocks - 1;
            while (m_num_blocks--) {
                pod_allocator<byte>::deallocate(blk->data, blk->size);
                --blk;
            }
            pod_allocator<block_type>::deallocate(m_blocks, m_max_blocks);
        }

        m_num_blocks = 0;
        m_max_blocks = 0;
        m_blocks = 0;
        m_buf_ptr = 0;
        m_remain_size = 0;
#if _DEBUG
        m_all_mem = 0;
#endif
    }

#if _DEBUG
    unsigned int all_mem_used(void) const { return m_all_mem;}
#endif

private:
    block_allocator(const block_allocator&);
    block_allocator& operator=(block_allocator&);

    void allocate_block(unsigned int size)
    {
        if (size < m_block_size) 
            size = m_block_size;

        if (m_num_blocks >= m_max_blocks) {
            block_type* new_blocks = pod_allocator<block_type>::allocate(m_max_blocks + m_block_ptr_inc);

#if _DEBUG
            m_all_mem += sizeof(block_type)*(m_block_ptr_inc);
#endif

            if (m_blocks) {
                memcpy(new_blocks, m_blocks, m_num_blocks * sizeof(block_type));
                pod_allocator<block_type>::deallocate(m_blocks, m_max_blocks);
            }

            m_blocks = new_blocks;
            m_max_blocks += m_block_ptr_inc;
        }

        m_blocks[m_num_blocks].size = size;
        m_blocks[m_num_blocks].data = m_buf_ptr = pod_allocator<byte>::allocate(size);

        m_num_blocks++;
        m_remain_size = size;

#if _DEBUG
        m_all_mem += m_remain_size;
#endif
    }

    unsigned int m_block_size;
    unsigned int m_block_ptr_inc;
    unsigned int m_num_blocks;
    unsigned int m_max_blocks;
    block_type*  m_blocks;
    byte*        m_buf_ptr;
    unsigned int m_remain_size;
#if _DEBUG
    unsigned int m_all_mem;
#endif
};

}
#endif/*_DATA_VECTOR_H_*/

