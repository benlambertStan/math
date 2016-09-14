#include <stan/math/rev/mat.hpp>
#include <gtest/gtest.h>
#include <test/unit/math/rev/mat/fun/expect_matrix_eq.hpp>
#include <test/unit/math/rev/mat/fun/util.hpp>

TEST(MathMatrix, matrix_exp_1x1) {
       
    stan::math::matrix_v m1(1,1), m2(1,1), m1_exp;
    m1 << 0;
    m2 << 1;
    m1_exp = matrix_exp(m1);
    expect_matrix_eq(m2, m1_exp);
    
    AVEC x = createAVEC(m1(0,0));
    VEC g;
    m1_exp(0,0).grad(x,g);
    EXPECT_FLOAT_EQ(m1_exp(0,0).val(), g[0]); 
    
}

TEST(MathMatrix, matrix_exp_2x2) {
    
    using stan::math::matrix_v;
    
    // example from Moler & Van Loan, 2003
    for (size_t k = 0; k < 2; k++) {
    	for (size_t l = 0; l < 2; l++) {
    				 
    		AVAR a = -1.0, b = -17.0;
			
			matrix_v m1(2,2), m2(2,2), m1_exp;
    		m1 << -2*a + 3*b, 1.5*a - 1.5*b, -4*a + 4*b, 3*a - 2*b;
   			m2 << -.735759, .551819, -1.471518, 1.103638;
    		m1_exp = matrix_exp(m1);
    		expect_matrix_eq(m2, m1_exp);
    		
    		matrix_v dm1_exp_da(2,2), dm1_exp_db(2,2);
    		AVAR exp_a = exp(a), exp_b = exp(b);
    		dm1_exp_da << -2 * exp_a, 1.5 * exp_a, -4 * exp_a, 3 * exp_a;
    		dm1_exp_db << 3 * exp_b, -1.5 * exp_b, 4 * exp_b, -2 * exp_b;
    
    		AVEC x = createAVEC(a, b);
    		VEC g;
    		m1_exp(k, l).grad(x, g);
    		EXPECT_FLOAT_EQ(dm1_exp_da(k, l).val(), g[0]);
    		EXPECT_FLOAT_EQ(dm1_exp_db(k, l).val(), g[1]);
    	}
    }
 
}

TEST(MathMatrix, matrix_exp_3x3) {

	using stan::math::matrix_v;

	for (size_t k = 0; k < 3; k++) {
		for (size_t l = 0; l < 3; l++) {

			AVAR a = -1.0, b = 2.0, c = 1.0;
			
			matrix_v m1(3,3), m2(3,3), m1_exp;
			m1 << -24*a +40*b - 15*c, 18*a - 30*b + 12*c, 5*a - 8*b + 3*c,
    		      20*b - 20*c, -15*b + 16*c, -4*b +4*c,
    		      -120*a + 120*b, 90*a - 90*b, 25*a - 24*b;
    		m2 << 245.95891, -182.43047, -49.11821,
    			  93.41549, -67.3433, -18.68310,
    			  842.54120, -631.90590, -168.14036;
    		m1_exp = matrix_exp(m1);
    		expect_matrix_eq(m2, m1_exp);
    		
    		matrix_v dm1_exp_da(3,3), dm1_exp_db(3,3), dm1_exp_dc(3,3);
    		AVAR exp_a = exp(a), exp_b = exp(b), exp_c = exp(c);
    		dm1_exp_da << -24 * exp_a, 18 * exp_a, 5 * exp_a,
    					  0, 0, 0, 
    					  -120 * exp_a, 90 * exp_a, 25 * exp_a;
    		dm1_exp_db << 40 * exp_b, -30 * exp_b, -8 * exp_b,
    					  20 * exp_b, -15 * exp_b, -4 * exp_b,
    					  120 * exp_b, -90 * exp_b, -24 * exp_b;
    		dm1_exp_dc << -15 * exp_c, 12 * exp_c, 3 * exp_c,
    					  -20 * exp_c, 16 * exp_c, 4 * exp_c,
    					  0, 0, 0;
    		
    		AVEC x = createAVEC(a, b, c);
    		VEC g;
    		m1_exp(k, l).grad(x, g);
    		EXPECT_NEAR(dm1_exp_da(k, l).val(), g[0], 1e-9);
    		EXPECT_NEAR(dm1_exp_db(k, l).val(), g[1], 1e-9);
    		EXPECT_NEAR(dm1_exp_dc(k, l).val(), g[2], 1e-9);
    	}
    }   					   

}

TEST(MathMatrix, matrix_exp_exceptions) {
    stan::math::matrix_v m1(0,0), m2(1,2);
    m2 << 1, 2;
    
    EXPECT_THROW(matrix_exp(m1), std::invalid_argument);
    EXPECT_THROW(matrix_exp(m2), std::invalid_argument);
}