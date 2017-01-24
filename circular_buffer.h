/* 
 * File:   circular_buffer.h
 * Author: Kappel
 *
 * Created on November 3, 2016, 6:23 PM
 */

#ifndef CIRCULAR_BUFFER_H
#define	CIRCULAR_BUFFER_H

/**
 * Minimal implementation of an iterable circular buffer.
 * 
 * @note resize erases all values and sets the whole vector to defaults.
 */
template<typename T>
class CircularBuffer
{
public:
    /**
     * Constant iterator class.
     */
    class const_iterator
    {
    public:
        /**
         * Copy constructor.
         */
        const_iterator(const const_iterator &src)
        : ptr_(src.ptr_),
          begin_(src.begin_),
          end_(src.end_)
        {}
        
        /**
         * Iterator increment. Wraps around at limits of the buffer.
         */        
        inline
        const_iterator &operator++()
        {
            if(++ptr_==end_)
                ptr_=begin_;
            return *this;
        }

        /**
         * Iterator decrement. Wraps around at limits of the buffer.
         */
        inline
        const_iterator &operator--()
        {
            if(ptr_==begin_)
                ptr_=end_-1;
            else
                ptr_--;
            return *this;
        }

        /**
         * Iterator dereferencing operator. Returns a constant reference
         * to the buffer content at the current location.
         */
        inline
        T const &operator*() const
        {
            return *ptr_;
        }
        
    private:
        friend class CircularBuffer;
        
        /**
         * Constructor.
         */
        const_iterator(size_t offs, T const *mem, size_t size)
        : ptr_(&mem[offs]),
          begin_(mem),
          end_(&mem[size])
        {}
        
        T const *ptr_;
        T const *begin_;
        T const *end_;
    };
    
    /**
     * Default constructor.
     */
    CircularBuffer()
    : mem_(0),
      size_(0)
    {}

    /**
     * Copy constructor.
     */
    CircularBuffer(const CircularBuffer &src)
    : mem_(0),
      size_(src.size_)
    {
        if (src.size_>0)
        {
            mem_ = new T[size_];
            for (size_t i=0;i<size_;i++)
                mem_[i] = src.mem_[i];
        }
    }
    
    /**
     * Destructor.
     */    
    ~CircularBuffer()
    {
        delete mem_;
    }
    
    /**
     * Change the size of the buffer to new value. All its content gets
     * overwritten with v.
     */
    void resize(size_t new_size, T v)
    {
        delete mem_;
        if (new_size>0)
        {
            mem_ = new T[new_size];
            for (size_t i=0;i<new_size;i++)
                mem_[i] = v;
            size_ = new_size;
        }
    }
    
    /**
     * Returns the value of the buffer at the given location. Location index
     * wraps around at the limits of the buffer.
     */    
    T &operator[](size_t at)
    {
        return mem_[get_index(at)];
    }
    
    /**
     * Returns a constant iterator to read from the buffer at the given
     * location. Location index wraps around at the limits of the buffer.
     */    
    const_iterator get(size_t at) const
    {
        return const_iterator(get_index(at),mem_,size_);
    }

    /**
     * Returns the current size of the buffer.
     */
    size_t size() const
    {
        return size_;
    }
    
private:
    /**
     * Computes the index to the buffer.
     */    
    inline
    size_t get_index(size_t at) const
    {
        return at % size_;
    }
    
    T *mem_;
    size_t size_;
};


#endif	/* CIRCULAR_BUFFER_H */
