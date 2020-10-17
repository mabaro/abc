#pragma once

#include "abc/core.hpp"
#include "abc/debug.hpp"

#include <atomic>

#define ABC_TESTING_ENABLED
#ifdef ABC_TESTING_ENABLED
#pragma message( \
    "ABC_TESTING is enabled. Some protected members are now public for testing purposes.")
#else
#endif

namespace abc
{
  //////////////////////////////////////////////////////////////////////////

  template <typename T>
  using shared_ptr = std::shared_ptr<T>;
  template <typename T>
  using weak_ptr = std::weak_ptr<T>;

  //////////////////////////////////////////////////////////////////////////

  template <typename T>
  class lendable;
  template <typename T>
  class unique_ptr;
  template <typename T>
  class unique_ref;
  template <typename T>
  class lent_ref;
  template <typename T>
  class lent_ptr;

  namespace detail
  {
    //////////////////////////////////////////////////////////////////////////

    class pointer_control_block
    {
      using ref_count_t = long;

      //volatile long references = 1;
      std::atomic<int32_t> references = 1;
      bool isFreed = false;

    public:
      inline void set_freed() { isFreed = true; }
      inline bool is_freed() { return isFreed; }
      inline ref_count_t add_reference() { return ++references; }
      inline ref_count_t remove_reference()
      {
        ABC_ASSERT(references > 0);
        return --references;
      }
      inline ref_count_t reference_count() { return references; }
      inline ref_count_t has_references() { return references > 0; }
    };

    template <typename T>
    class unique_pointer_internal;
    template <typename T>
    class lent_pointer_internal;
    template <typename T>
    class lendable_internal;

    template <typename T>
    class unique_pointer_promoter;
    template <typename T>
    class lent_pointer_promoter;

    //////////////////////////////////////////////////////////////////////////
    // UNIQUE POINTERS
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class unique_pointer_internal : public abc::noncopyable
    {
      using this_t = unique_pointer_internal<T>;
      using value_t = T;

    public:
      inline T &operator*() const { return *m_pointer; }
      inline T *operator->() const { return m_pointer; }
      inline explicit operator bool() const { return m_pointer != nullptr; }

      inline T *get_raw_ptr() { return m_pointer; }

      inline void reset(T *pointer = nullptr)
      {
        static_assert(!std::is_polymorphic<T>::value || std::has_virtual_destructor<T>::value,
                      "Trying to delete a polymorphic object through a non-virtual destructor.\n");

        if (m_pointer != pointer)
        {
          // ABC_ASSERT_MSG(m_pointer == nullptr || m_controlBlock->references == 0, format("There are
          // dangling lent pointers: {}", m_controlBlock->references));
          if (m_pointer)
          {
            delete m_pointer;
            m_pointer = nullptr;
          }
          if (m_controlBlock)
          {
            const auto references = m_controlBlock->remove_reference();
            if (references == 0)
            { // otherwise let it dangle and notify
              delete m_controlBlock;
            }
            else
            {
              ABC_FAIL_RELEASE("UNIQUE_PTR_DANGLING: unique_ptr deleted with {} references",
                               references);
            }
            m_controlBlock = nullptr;
          }

          m_pointer = pointer;
          m_controlBlock = m_pointer ? new pointer_control_block() : nullptr;
        }
      }

      template <typename T2>
      inline void reset(T2 *pointer = nullptr)
      {
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_unique_cast.");
        reset(static_cast<T>(pointer));
      }

    public:
      virtual ~unique_pointer_internal()
      {
        static_assert(!std::is_polymorphic<T>::value || std::has_virtual_destructor<T>::value,
                      "Trying to delete a polymorphic object through a non-virtual destructor.\n");
        reset();
      }

      inline explicit unique_pointer_internal(bool) =
          delete; // avoid implicit cast from nullptr on some compilers
      inline unique_pointer_internal(const this_t &other) = delete;

      inline unique_pointer_internal(this_t &&other)
          : m_pointer(std::move(other.m_pointer)), m_controlBlock(std::move(other.m_controlBlock))
      {
        other.initialize();
      }
      template <typename T2>
      inline unique_pointer_internal(unique_pointer_internal<T2> &&other)
          : m_pointer(static_cast<T *>(std::move(other.m_pointer))),
            m_controlBlock(std::move(other.m_controlBlock))
      {
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_unique_cast.");
        other.initialize();
      }

      inline explicit unique_pointer_internal(T *pointer)
          : m_pointer(pointer), m_controlBlock(pointer ? new pointer_control_block() : nullptr) {}
      template <typename T2>
      inline explicit unique_pointer_internal(T2 *pointer)
          : m_pointer(pointer), m_controlBlock(pointer ? new pointer_control_block() : nullptr)
      {
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_unique_cast.");
      }

      template <typename T2>
      inline unique_pointer_internal<T> operator=(const std::nullptr_t &)
      {
        ABC_ASSERT(
            (m_pointer == nullptr && m_controlBlock == nullptr) || m_controlBlock->has_references(),
            format("Assignment operator is gonna destroy a unique pointer, leaving '{}' dangling "
                   "lent_pointers",
                   m_controlBlock->reference_count()));
        reset(nullptr);

        return *this;
      }
      inline this_t &operator=(this_t &&other)
      {
        ABC_ASSERT(
            (m_pointer == nullptr && m_controlBlock == nullptr) || m_controlBlock->has_references(),
            format("Assignment operator is gonna destroy a unique pointer, leaving '{}' dangling "
                   "lent_pointers",
                   m_controlBlock->reference_count()));
        reset(nullptr);

        m_pointer = std::move(other.m_pointer);
        m_controlBlock = std::move(other.m_controlBlock);

        other.initialize();
        ABC_ASSERT(other.m_pointer == nullptr);
        ABC_ASSERT(other.m_controlBlock == nullptr);

        return *this;
      }
      template <typename T2>
      inline this_t &operator=(unique_pointer_internal<T2> &&other)
      {
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_unique_cast.");

        ABC_ASSERT(
            (m_pointer == nullptr && m_controlBlock == nullptr) || m_controlBlock->has_references(),
            format("Assignment operator is gonna destroy a unique pointer, leaving '{}' dangling "
                   "lent_pointers",
                   m_controlBlock->reference_count()));
        reset(nullptr);

        m_pointer = std::move(other.m_pointer);
        m_controlBlock = std::move(other.m_controlBlock);

        other.initialize();
        ABC_ASSERT(other.m_pointer == nullptr);
        ABC_ASSERT(other.m_controlBlock == nullptr);

        return *this;
      }

    public:
      // template<typename T2> inline bool operator==(const unique_ptr<T2>& other) { return m_pointer ==
      // other.m_pointer; } template<typename T2> inline bool operator==(const unique_ref<T2>& other) {
      // return m_pointer == other.m_pointer; } template<typename T2> inline bool operator==(const
      // lent_ptr<T2>& other) { return m_pointer == other.m_pointer; } template<typename T2> inline bool
      // operator==(const lent_ref<T2>& other) { return m_pointer == other.m_pointer; }
      // template<typename T2> inline bool operator==(const lendable<T2>& other) { return m_pointer ==
      // other.get_raw_ptr(); } template<typename T2> inline bool operator==(const std::nullptr_t& nul)
      // { return m_pointer == nullptr; }

    protected: // lent_pointer_checked API
      inline void initialize()
      {
        m_pointer = nullptr;
        m_controlBlock = nullptr;
      }
      inline void add_reference()
      {
        ABC_ASSERT(m_controlBlock,
                   "Accessing a dangling lent_pointer, it's unique_point has been destroyed");
        m_controlBlock->add_reference();
      }
      inline void remove_reference()
      {
        ABC_ASSERT(m_controlBlock,
                   "Accessing a dangling lent_pointer, it's unique_point has been destroyed");
        m_controlBlock->remove_reference();
      }

#ifdef ABC_TESTING_ENABLED
    public:
#else
    protected:
#endif // ABC_TESTING_ENABLED
      T *m_pointer = nullptr;
      pointer_control_block *m_controlBlock = nullptr;

    protected:
      template <typename T2>
      friend class unique_pointer_internal;
      template <typename T2>
      friend class lent_pointer_internal;

      template <typename T2>
      friend class detail::lent_pointer_promoter;
      template <typename T2>
      friend class detail::unique_pointer_promoter;

    protected: // comparisons
      template <typename T1>
      friend bool operator==(const unique_pointer_internal<T1> &lhs, std::nullptr_t);
      template <typename T1>
      friend bool operator==(std::nullptr_t, const unique_pointer_internal<T1> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const unique_pointer_internal<T1> &lhs,
                             const unique_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const unique_pointer_internal<T1> &lhs, const T2 *rhs);
      template <typename T1, typename T2>
      friend bool operator==(const T1 *lhs, const unique_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const unique_pointer_internal<T1> &lhs, const T2 rhs);
      template <typename T1, typename T2>
      friend bool operator==(const T1 lhs, const unique_pointer_internal<T2> &rhs);

      template <typename T1>
      friend bool operator!=(const unique_pointer_internal<T1> &lhs, std::nullptr_t);
      template <typename T1>
      friend bool operator!=(std::nullptr_t, const unique_pointer_internal<T1> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const unique_pointer_internal<T1> &lhs,
                             const unique_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const unique_pointer_internal<T1> &lhs, const T2 *rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const T1 *lhs, const unique_pointer_internal<T2> &rhs);
    };

    template <typename T1>
    inline bool operator==(const unique_pointer_internal<T1> &lhs, std::nullptr_t)
    {
      return lhs.m_pointer == nullptr;
    }
    template <typename T1>
    inline bool operator==(std::nullptr_t, const unique_pointer_internal<T1> &rhs)
    {
      return nullptr == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const unique_pointer_internal<T1> &lhs,
                           const unique_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const unique_pointer_internal<T1> &lhs, const T2 *rhs)
    {
      return lhs.m_pointer == rhs;
    }
    template <typename T1, typename T2>
    inline bool operator==(const T1 *lhs, const unique_pointer_internal<T2> &rhs)
    {
      return lhs == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const unique_pointer_internal<T1> &lhs, const T2 rhs)
    {
      return lhs.m_pointer == rhs;
    }
    template <typename T1, typename T2>
    inline bool operator==(const T1 lhs, const unique_pointer_internal<T2> &rhs)
    {
      return lhs == rhs.m_pointer;
    }

    template <typename T1>
    inline bool operator!=(const unique_pointer_internal<T1> &lhs, std::nullptr_t)
    {
      return lhs.m_pointer != nullptr;
    }
    template <typename T1>
    inline bool operator!=(std::nullptr_t, const unique_pointer_internal<T1> &rhs)
    {
      return nullptr != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const unique_pointer_internal<T1> &lhs,
                           const unique_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const unique_pointer_internal<T1> &lhs, const T2 *rhs)
    {
      return lhs.m_pointer != rhs;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const T1 *lhs, const unique_pointer_internal<T2> &rhs)
    {
      return lhs != rhs.m_pointer;
    }

    //////////////////////////////////////////////////////////////////////////
  } // namespace detail

  template <typename T>
  class unique_ref : public detail::unique_pointer_internal<T>
  {
    using parent_t = detail::unique_pointer_internal<T>;

    inline unique_ref(parent_t &&other) : parent_t(std::move(other)) {}
    template <typename T2>
    inline unique_ref(detail::unique_pointer_internal<T2> &&other) : parent_t(std::move(other)) {}

  public:
    unique_ref() = delete;
    unique_ref(std::nullptr_t) = delete;
    unique_ref(const unique_ref<T> &) = delete;
    template <typename T2>
    unique_ref(const unique_ptr<T2> &) = delete;
    template <typename T2>
    unique_ref(const unique_ref<T2> &) = delete;

    template <typename T2>
    unique_ref(T2 *pointer) : parent_t(static_cast<T *>(pointer))
    {
      static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                    "Trying to assign a const object to a non-const object");
      ABC_ASSERT(pointer != nullptr);
    }
    template <typename T2>
    unique_ref(unique_ref<T2> &&other) : parent_t(std::move(other))
    {
      static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                    "Trying to assign a const object to a non-const object");
    }

  protected:
    friend class detail::unique_pointer_promoter<T>;
  };

  ////////////////////////////////////////////////////////////

  template <typename T>
  class unique_ptr : public detail::unique_pointer_internal<T>
  {
    using parent_t = detail::unique_pointer_internal<T>;

  public:
    inline unique_ptr() : parent_t(nullptr) {}
    inline explicit unique_ptr(T *pointer) : parent_t(pointer) {}
    inline unique_ptr(unique_ptr<T> &&other) : parent_t(std::move(other)) {}
    inline unique_ptr(unique_ref<T> &&other) : parent_t(std::move(other)) {}

    unique_ptr<T> &operator=(const std::nullptr_t &nul)
    {
      parent_t::reset();
      return *this;
    }
    unique_ptr<T> &operator=(unique_ptr<T> &&other)
    {
      parent_t::operator=(std::move(other));
      return *this;
    }
    template <typename T2>
    unique_ptr<T> &operator=(unique_ptr<T2> &&other)
    {
      parent_t::operator=(std::move(other));
      return *this;
    }
    unique_ptr<T> &operator=(unique_ref<T> &&other)
    {
      parent_t::operator=(std::move(other));
      return *this;
    }
    template <typename T2>
    unique_ptr<T> &operator=(unique_ref<T2> &&other)
    {
      parent_t::operator=(std::move(other));
      return *this;
    }

    using parent_t::get_raw_ptr;
  };

  //////////////////////////////////////////////////////////////////////////

  template <typename T, typename... Args>
  inline unique_ref<T> make_unique(Args &&... args)
  {
    return unique_ref<T>(new T(std::forward<Args>(args)...));
  }

  //////////////////////////////////////////////////////////////////////////
  /// LENDABLE
  //////////////////////////////////////////////////////////////////////////

  namespace detail
  {
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class lendable_internal
    {
    public:
      template <typename... Args>
      lendable_internal(Args &&... args)
          : m_object(std::forward<Args>(args)...), m_controlBlock(new pointer_control_block()) {}

      ~lendable_internal()
      {
        m_controlBlock->set_freed();
        const size_t references = m_controlBlock->remove_reference();
        if (references == 0)
        {
          delete m_controlBlock;
        }
        else
        {
          ABC_FAIL("LENDABLE_DANGLING: lendable deleted with {} dangling references.", references);
        }
      }

      inline T *get_raw_ptr() const
      {
        return const_cast<typename std::remove_const<T *>::type>(&m_object);
      }

#ifdef ABC_TESTING_ENABLED
    public:
#else
    protected:
#endif // ABC_TESTING_ENABLED
      T m_object;
      pointer_control_block *m_controlBlock = nullptr;

    protected:
      template <typename T2>
      friend class lent_pointer_internal;
      template <typename T2>
      friend class lendable_helper;
    };

    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class lendable_helper;

    //////////////////////////////////////////////////////////////////////////
  } // namespace detail

  //////////////////////////////////////////////////////////////////////////

  template <typename T>
  class lendable : public detail::lendable_internal<T>
  {
  protected:
    using parent_t = detail::lendable_internal<T>;

  public:
    using this_t = lendable<T>;
    using value_t = T;

  public:
    inline ~lendable() = default;

    template <typename... Args>
    inline lendable(Args &&... args) : parent_t(std::forward<Args>(args)...) {}

    inline lendable(const this_t &other) : parent_t::m_object(other->m_object) {}
    inline lendable(this_t &&other) : parent_t(std::forward(other)) {}

    inline lendable(const T &object) : parent_t::m_object(object) {}
    inline lendable(T &&object) : parent_t(std::forward<T>(object)) {}

    inline this_t &operator=(const this_t &other)
    {
      parent_t::m_object = other.m_object;
      return *this;
    }
    inline this_t &operator=(const T &object)
    {
      parent_t::m_object = object;
      return *this;
    }
    inline this_t &operator=(this_t &&other)
    {
      parent_t::m_object = std::move(other.m_object);
      return *this;
    }
    inline this_t &operator=(T &&object)
    {
      parent_t::m_object = std::move(object);
      return *this;
    }

  public:
    inline T *operator->() { return &parent_t::m_object; }
    inline const T *operator->() const { return &parent_t::m_object; }
    inline T &operator*() { return parent_t::m_object; }
    inline const T &operator*() const { return parent_t::m_object; }

  protected:
    template <typename U>
    friend class detail::lendable_helper;
    template <typename U>
    friend class detail::lent_pointer_internal;
  };

  //////////////////////////////////////////////////////////////////////////
  // LENT POINTERS
  //////////////////////////////////////////////////////////////////////////

  namespace detail
  {
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class lent_pointer_internal
    {
      using this_t = lent_pointer_internal<T>;
      using value_t = T;

    protected:
      T *m_pointer = nullptr;
      pointer_control_block *m_controlBlock = nullptr;

    protected:
      virtual ~lent_pointer_internal() { reset(); }

      lent_pointer_internal() {}
      lent_pointer_internal(const this_t &other)
          : m_pointer(other.m_pointer), m_controlBlock(other.m_controlBlock)
      {
        add_reference();
      }
      template <typename T2>
      lent_pointer_internal(const lent_pointer_internal<T2> &other)
          : m_pointer(other.m_pointer), m_controlBlock(other.m_controlBlock)
      {
        add_reference();
      }
      template <typename T2>
      lent_pointer_internal(T2 *pointer, pointer_control_block *controlBlock)
          : m_pointer(pointer), m_controlBlock(controlBlock)
      {
        add_reference();
      }
      template <typename T2>
      explicit lent_pointer_internal(const unique_ref<T2> &uniqueReference)
          : m_pointer(uniqueReference.m_pointer), m_controlBlock(uniqueReference.m_controlBlock)
      {
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_lent_cast.");
        add_reference();
      }
      template <typename T2>
      explicit lent_pointer_internal(const unique_ptr<T2> &uniqueReference)
          : m_pointer(uniqueReference.m_pointer), m_controlBlock(uniqueReference.m_controlBlock)
      {
        static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                      "Trying to assign a const object to a non-const object");
        static_assert((std::is_class<T>::value == false || std::is_base_of<T, T2>::value),
                      "Only upcast conversions are implicit. Use static_lent_cast.");
        add_reference();
      }

      template <typename T2>
      lent_pointer_internal(lent_pointer_internal<T2> &&other)
          : m_pointer(std::move(other.m_pointer)), m_controlBlock(std::move(other.m_controlBlock))
      {
        static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                      "Trying to assign a const object to a non-const object");
        add_reference();
      }

    public:
      this_t &operator=(std::nullptr_t) { reset(); }
      this_t &operator=(const this_t &other)
      {
        remove_reference();
        m_pointer = other.m_pointer;
        m_controlBlock = other.m_controlBlock;
        add_reference();

        return *this;
      }
      this_t &operator=(const unique_pointer_internal<T> &unique)
      {
        remove_reference();
        m_pointer = unique.m_pointer;
        m_controlBlock = unique.m_controlBlock;
        add_reference();

        return *this;
      }

      this_t &operator=(this_t &&other)
      {
        m_pointer = std::move(other.m_pointer);
        m_controlBlock = std::move(other.m_controlBlock);
        other.initialize();

        return *this;
      }

      void reset()
      {
        remove_reference();
        m_controlBlock = nullptr;
        m_pointer = nullptr;
      }

    public:
      inline T &operator*() const
      {
        if (m_pointer)
        {
          return *m_pointer;
        }
        else
        {
          ABC_FAIL("LENT_REF_NULL_ACCESS: Accessing a null pointer");
        }
      }
      inline T *operator->() const { return m_pointer; }
      inline explicit operator bool() const { return m_pointer != nullptr; }

      inline void swap(lent_pointer_internal<T> &ref)
      {
        std::swap(m_pointer, ref.m_pointer);
        std::swap(m_controlBlock, ref.m_controlBlock);
      }

    protected:
      void initialize()
      {
        m_pointer = nullptr;
        m_controlBlock = nullptr;
      }
      void add_reference()
      {
        if (m_controlBlock)
        {
          ABC_ASSERT(m_pointer);
          m_controlBlock->add_reference();
        }
      }
      void remove_reference()
      {
        if (m_controlBlock)
        {
          ABC_ASSERT(m_pointer);
          m_controlBlock->remove_reference();
        }
      }

    protected:
      template <typename U>
      friend class lent_pointer_internal;
      template <typename U>
      friend class lent_pointer_promoter;
      template <typename U>
      friend class unique_pointer_internal;
      template <typename U>
      friend class lendable_internal;
      template <typename U>
      friend class lendable_helper;

    protected:
      template <typename T1>
      friend bool operator==(const lent_pointer_internal<T1> &lhs, std::nullptr_t);
      template <typename T1>
      friend bool operator==(std::nullptr_t, const lent_pointer_internal<T1> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const lent_pointer_internal<T1> &lhs,
                             const detail::lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const lent_pointer_internal<T1> &lhs, const T2 *rhs);
      template <typename T1, typename T2>
      friend bool operator==(const T1 *lhs, const lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const unique_pointer_internal<T1> &lhs,
                             const lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator==(const lent_pointer_internal<T1> &lhs,
                             const unique_pointer_internal<T2> &rhs);

      template <typename T1>
      friend bool operator!=(const lent_pointer_internal<T1> &lhs, std::nullptr_t);
      template <typename T1>
      friend bool operator!=(std::nullptr_t, const lent_pointer_internal<T1> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const lent_pointer_internal<T1> &lhs,
                             const detail::lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const lent_pointer_internal<T1> &lhs, const T2 *rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const T1 *lhs, const lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const unique_pointer_internal<T1> &lhs,
                             const lent_pointer_internal<T2> &rhs);
      template <typename T1, typename T2>
      friend bool operator!=(const lent_pointer_internal<T1> &lhs,
                             const unique_pointer_internal<T2> &rhs);
    };

    template <typename T1>
    inline bool operator==(const lent_pointer_internal<T1> &lhs, std::nullptr_t)
    {
      return lhs.m_pointer == nullptr;
    }
    template <typename T1>
    inline bool operator==(std::nullptr_t, const lent_pointer_internal<T1> &rhs)
    {
      return nullptr == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const lent_pointer_internal<T1> &lhs, const lent_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const lent_pointer_internal<T1> &lhs, const T2 *rhs)
    {
      return lhs.m_pointer == rhs;
    }
    template <typename T1, typename T2>
    inline bool operator==(const T1 *lhs, const lent_pointer_internal<T2> &rhs)
    {
      return lhs == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const unique_pointer_internal<T1> &lhs,
                           const lent_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer == rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator==(const lent_pointer_internal<T1> &lhs,
                           const unique_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer == rhs.m_pointer;
    }

    template <typename T1>
    inline bool operator!=(const lent_pointer_internal<T1> &lhs, std::nullptr_t)
    {
      return lhs.m_pointer != nullptr;
    }
    template <typename T1>
    inline bool operator!=(std::nullptr_t, const lent_pointer_internal<T1> &rhs)
    {
      return nullptr != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const lent_pointer_internal<T1> &lhs, const lent_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const lent_pointer_internal<T1> &lhs, const T2 *rhs)
    {
      return lhs.m_pointer != rhs;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const T1 *lhs, const lent_pointer_internal<T2> &rhs)
    {
      return lhs != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const unique_pointer_internal<T1> &lhs,
                           const lent_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer != rhs.m_pointer;
    }
    template <typename T1, typename T2>
    inline bool operator!=(const lent_pointer_internal<T1> &lhs,
                           const unique_pointer_internal<T2> &rhs)
    {
      return lhs.m_pointer != rhs.m_pointer;
    }

    //////////////////////////////////////////////////////////////////////////
  } // namespace detail

  template <typename T>
  class lent_ptr;

  template <typename T>
  class lent_ref : public detail::lent_pointer_internal<T>
  {
    using parent_t = detail::lent_pointer_internal<T>;
    using parent_t::reset; // lent_ref cannot be reset

  public:
    ~lent_ref() {}

    lent_ref(const lent_ref<T> &other) : parent_t(other) {}
    template <typename T2>
    lent_ref(const lent_ref<T2> &other) : parent_t(other) {}
    template <typename T2>
    lent_ref(const unique_ref<T2> &uniqueReference) : parent_t(uniqueReference)
    {
      static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                    "Trying to assign a const object to a non-const object");
    }

  protected:
    // for lend/promote helpers
    template <typename T2>
    lent_ref(const detail::lent_pointer_internal<T2> &object)
        : detail::lent_pointer_internal<T2>(object) {}

  protected:
    template <typename T2>
    friend class detail::lent_pointer_promoter;
    template <typename T2>
    friend class detail::lendable_helper;

    template <class T2>
    friend class lendable;
    template <class T2>
    friend class detail::lendable_internal;
  };

  template <typename T>
  class lent_ptr : public detail::lent_pointer_internal<T>
  {
    using parent_t = detail::lent_pointer_internal<T>;
    using this_t = lent_ptr<T>;
    using value_t = T;

  public:
    ~lent_ptr() {}

    lent_ptr() : parent_t() {}
    explicit lent_ptr(std::nullptr_t) : parent_t() {}
    lent_ptr(const lent_ptr<T> &other) : parent_t(other) {}
    template <typename T2>
    lent_ptr(const lent_ptr<T2> &other) : parent_t(other) {}
    template <typename T2>
    lent_ptr(const lent_ref<T2> &other) : parent_t(other) {}
    template <typename T2>
    lent_ptr(const unique_ref<T2> &uniqueReference) : parent_t(uniqueReference)
    {
      static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                    "Trying to assign a const object to a non-const object");
    }
    template <typename T2>
    lent_ptr(const unique_ptr<T2> &uniqueReference) : parent_t(uniqueReference)
    {
      static_assert(std::is_const<T>::value || !std::is_const<T2>::value,
                    "Trying to assign a const object to a non-const object");
    }
  };

  //////////////////////////////////////////////////////////////////////////
  /// LENDING AND PROMOTING
  //////////////////////////////////////////////////////////////////////////

  namespace detail
  {
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class unique_pointer_promoter
    {
    public:
      static inline unique_ref<T> promote(unique_ptr<T> &&other)
      {
        ABC_ASSERT(other);
        detail::unique_pointer_internal<T> temp(std::move(other));
        return unique_ref<T>(std::move(temp));
      }
    };

    template <typename T>
    class lent_pointer_promoter
    {
    public:
      template <typename T2>
      static inline lent_ref<T> promote(const lent_ptr<T2> &other)
      {
        ABC_ASSERT(other);
        return lent_ref<T>(other);
      }
    };

    template <typename T>
    constexpr bool always_fail()
    {
      return false;
    }

    //////////////////////////////////////////////////////////////////////////
  } // namespace detail

  template <typename T>
  inline unique_ref<T> promote_to_ref(unique_ptr<T> &&pointer)
  {
    ABC_ASSERT(pointer);
    if (pointer)
    {
      return detail::unique_pointer_promoter<T>::promote(std::move(pointer));
    }
    return unique_ref<T>(reinterpret_cast<T *>(0xDEAD));
  }
  template <typename T>
  inline unique_ref<T> promote_to_ref(unique_ref<T> &&pointer)
  {
    static_assert(detail::always_fail<T>, "Cannot promote to reference a reference");
    return unique_ref<T>(reinterpret_cast<T *>(0xDEAD));
  }

  template <typename T>
  inline lent_ref<T> promote_to_ref(const lent_ptr<T> &pointer)
  {
    ABC_ASSERT(pointer != nullptr);
    if (pointer)
    {
      return detail::lent_pointer_promoter<T>::promote(pointer);
    }
    return unique_ref<T>(reinterpret_cast<T *>(0xDEAD));
  }
  template <typename T>
  inline lent_ref<T> promote_to_ref(const lent_ref<T> &pointer)
  {
    static_assert(detail::always_fail<T>, "Cannot promote to reference a reference");
    return pointer;
  }

  namespace detail
  {
    //////////////////////////////////////////////////////////////////////////

    template <bool, class L, class R>
    struct conditional_type_selector
    {
      typedef R type;
    };

    template <class L, class R>
    struct conditional_type_selector<true, L, R>
    {
      typedef L type;
    };

    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class lendable_helper
    {
    public:
      template <typename T2>
      static lent_ref<
          typename std::enable_if<!std::is_const<T2>::value || std::is_const<T>::value, T>::type>
      cast(const lendable<T2> &object)
      {
        // detail::lendable_internal<T2>& lendableIntenal =
        // static_cast<detail::lendable_internal<T2>>(lendableReference); T* object =
        // lendableIntenal.get_raw_ptr(); T2 object = lendableReference.m_object; pointer_control_block*
        // controlBlock = lendableReference.m_controlBlock; return
        // lent_ref<T>(lent_pointer_internal<T>(lendableReference));

        // return
        // lent_ref<T>(lent_pointer_checked<T>(static_cast<conditional_type_selector<std::is_const<T2>::value,
        //										   const std::remove_const<T*>::type, T*>::type>(&i_lendable.m_object),
        //i_lendable.m_control_block));

        T *ptr = const_cast<T *>(static_cast<const T *>(object.get_raw_ptr()));
        pointer_control_block *controlBlock = object.m_controlBlock;
        return lent_ref<T>(detail::lent_pointer_internal<T>(ptr, controlBlock));
      }
    };

    //////////////////////////////////////////////////////////////////////////
  } // namespace detail

  // template<typename T>
  // lent_ptr<typename detail::conditional_type_selector<std::is_const<T>::value, const typename
  // std::remove_const<T>::type, typename std::remove_const<T>::type>::type> lend(const unique_ptr<T>&
  // unique)
  //{
  //	return lent_ptr<T>(unique);
  //}
  template <typename T>
  lent_ptr<T> lend(const unique_ptr<T> &unique)
  {
    return lent_ptr<T>(unique);
  }
  template <typename T>
  lent_ref<T> lend(const unique_ref<T> &unique)
  {
    return lent_ref<T>(unique);
  }
  template <typename T>
  lent_ref<T> lend(const lendable<T> &object)
  {
    return detail::lendable_helper<T>::template cast<T>(object);
  }

  //////////////////////////////////////////////////////////////////////////
} // namespace abc
