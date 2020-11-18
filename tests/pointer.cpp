#include "doctest/doctest.h"

// Enable this to expose internal members in abc (i.e., ptrs control block)
//#define ABC_TESTING_ENABLED

#ifdef ABC_TESTING_ENABLED
#  define ABC_CHECK(X) CHECK(X)
#else
#  define ABC_CHECK(X)
#endif

#include "abc/core.hpp"
#include "abc/pointer.hpp"

template <typename T>
void test_unique_ptr() {
  using namespace abc;
  using type_t = T;
  {
    unique_ptr<type_t> unique_int = make_unique<type_t>(1);
    ABC_CHECK(unique_int.m_controlBlock->reference_count() == 1);
    {  // unique_ptr
      CHECK(unique_int.get_raw_ptr() == unique_int);
      CHECK(unique_int != nullptr);
      CHECK(make_unique<type_t>() != nullptr);
      CHECK(make_unique<type_t>(1) != nullptr);
      CHECK(make_unique<type_t>(1) != new type_t(1));
      CHECK(*make_unique<type_t>(1) == 1);
      CHECK(*make_unique<type_t>(1) != 2);
      CHECK(unique_int == lend(unique_int));
      CHECK(unique_int == promote_to_ref(lend(unique_int)));
    }
    ABC_CHECK(unique_int.m_controlBlock->reference_count() == 1);
    {  // move operator
      unique_ref<type_t> unique_ref = promote_to_ref(std::move(unique_int));
      unique_int                    = std::move(unique_ref);
    }
    ABC_CHECK(unique_int.m_controlBlock->reference_count() == 1);
    {  // lent_ptr
      lent_ptr<type_t> lent_int = unique_int;
      CHECK(lent_int != nullptr);
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
      {
        CHECK(lend(unique_int) != nullptr);
        lent_ptr<type_t> lent1 = unique_int;
        lent_ptr<type_t> lent2 = lend(unique_int);
        CHECK(lent1 == unique_int);
      }
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
      {
        CHECK(lend(unique_int) != nullptr);
        lent_ptr<const type_t> lent1 = lent_int;
        lent_ptr<const type_t> lent2 = lend(unique_int);
        lent_ptr<const type_t> lent4 = promote_to_ref(lend(unique_int));
        CHECK(lent1 == unique_int);
      }
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
    }
    ABC_CHECK(unique_int.m_controlBlock->reference_count() == 1);
    {  // lent_ref
      lent_ptr<type_t> lent_int = unique_int;
      CHECK(lent_int != nullptr);
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
      {
        // lent_ref<type_t> lent1 = unique_int; // forbidden
        // lent_ref<type_t> lent2 = lend(unique_int);// forbidden
        lent_ref<type_t> lent3 = promote_to_ref(lend(unique_int));
        lent_ref<type_t> lent4 = promote_to_ref(lent_int);
        CHECK(lent_int == lent4);
      }
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
      {
        lent_ptr<const type_t> lent5 = lent_int;
        lent_ref<const type_t> lent6 = promote_to_ref(lent_int);
        lent_ref<const type_t> lent7 = promote_to_ref(lend(unique_int));
        CHECK(lent7 == unique_int);
      }
      ABC_CHECK(unique_int.m_controlBlock->reference_count() == 2);
    }
    ABC_CHECK(unique_int.m_controlBlock->reference_count() == 1);
    unique_int = nullptr;  //== reset
  }
  {  // compilation errors
#if 0
		{// const-ness mismatch
			unique_ref<type_t> unique1 = make_unique<const type_t>(1);
			unique_ptr<type_t> unique2 = make_unique<const type_t>(1);
			lent_ptr<type_t> lent1 = make_unique<const type_t>(1);
			lent_ref<type_t> lent2 = make_unique<const type_t>(1);
		}

		{// ref-ptr mismatch
			lent_ref<type_t> lent2 = lent_ptr<type_t>(nullptr);
			lent_ref<type_t> ref1(nullptr); // forbidden
			lent_ref<type_t> ref2 = abc::promote_to_ref(abc::lent_ptr<type_t>(nullptr)); //forced crash
		}
		{// dangling assertion
			unique_ptr<const type_t> unique_const_int = make_unique<const type_t>(1);
			lent_ptr<type_t> lent1 = make_unique<type_t>(1);
			lent_ref<type_t> lent2 = make_unique<type_t>(1);
			lent_ref<const type_t> lent3 = abc::promote_to_ref(std::move(unique_const_int));
		}
#endif
  }
}

TEST_CASE("abc - unique_pointer") {
  using namespace abc;

  struct Dummy {
    Dummy() {}
    Dummy(int aa) : a(new int(aa)) {}
    Dummy(const Dummy& other) : a(new int(*other.a)) {}
    virtual ~Dummy() { delete a; }
    int* a = nullptr;
    bool operator==(const Dummy& other) const { return *a == *other.a; }
    bool operator!=(const Dummy& other) const { return *a != *other.a; }
  };
  struct Dummy2 : public Dummy {
    Dummy2() : Dummy() {}
    Dummy2(int aa) : Dummy(aa) {}
    int a2;
  };

  test_unique_ptr<int>();
  // can't instantiate undefined//test_unique_ptr<struct A>();
  test_unique_ptr<Dummy>();
  test_unique_ptr<Dummy2>();
}

TEST_CASE("abc - unique_pointer - move") {
  using namespace abc;

  struct MoveTester {
    static abc::unique_ref<int> transfer(abc::unique_ref<int>&& int0) { return std::move(int0); }
    static void sink(abc::unique_ref<int>&& int0) { auto death = std::move(int0); }
  };

  unique_ref<int> int0 = make_unique<int>(1);
  unique_ref<int> int1 = std::move(int0);
  ABC_CHECK(int0.get_raw_ptr() == nullptr);

  auto int2 = MoveTester::transfer(std::move(int1));
  ABC_CHECK(int1.get_raw_ptr() == nullptr);
  ABC_CHECK(int2.get_raw_ptr() != nullptr);

   MoveTester::sink(std::move(int2));
  ABC_CHECK(int2.get_raw_ptr() == nullptr);
}

TEST_CASE("abc - shared_pointer")
{
  using namespace abc;

  shared_ptr<int> i0(new int(1));
  shared_ptr<int> i1 = i0;
}
