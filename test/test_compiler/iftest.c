#include "iftest.h"
#include "yats.h"

SETUP_YATS();


static void test_if() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		BRF_8,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		POP,
		HALT
	};
	ASSERT_GEN_BC_EQ(expected, "if true { true; };");
}

static void test_ifelse() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		BRF_8,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		POP,
		BR_8,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_F,
		POP,
		HALT
	};
	ASSERT_GEN_BC_EQ(expected, "if true { true; } else { false; };");
}

static void test_ifelseelseif() {
	unsigned char expected[] = {
		0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		BRF_8,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_T,
		POP,
		BR_8,
		0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_F,
		BRF_8,
		0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		BCONST_F,
		POP,
		BR_8,
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		NCONST,
		POP,
		HALT
	};
	ASSERT_GEN_BC_EQ(expected, "if true { true; } elseif false { false; } else { undef; };");
}


int iftest(void) {
	test_if();
	test_ifelse();
	test_ifelseelseif();

	return __YASL_TESTS_FAILED__;
}
