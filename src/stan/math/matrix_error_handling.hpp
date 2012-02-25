#ifndef __STAN__MATH__MATRIX_ERROR_HANDLING_HPP__
#define __STAN__MATH__MATRIX_ERROR_HANDLING_HPP__

#include <limits>

#include <stan/math/boost_error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/math/error_handling.hpp>

#include <stan/math/matrix.hpp>

#include <boost/type_traits/common_type.hpp>

namespace stan { 

  namespace math {
    template <typename T_size1, typename T_size2, typename T_result = T_size1, class Policy = default_policy>
    inline bool check_size_match(const char* function,
                                 T_size1 i,
                                 T_size2 j,
                                 T_result* result = 0,
                                 const Policy& = Policy()) {
      using stan::math::policies::raise_domain_error;
      typedef typename boost::common_type<T_size1,T_size2>::type common_type;
      if (static_cast<common_type>(i) != static_cast<common_type>(j)) {
        std::ostringstream msg;
        msg << "i and j must be same.  Found i=%1%, j=" << j;
        T_result tmp = raise_domain_error<T_result,T_size1>(function,
                                                            msg.str().c_str(),
                                                            i,
                                                            Policy());
        if (result != 0)
          *result = tmp;
        return false;
      }
      return true;
    }
    
    /**
     * Return <code>true</code> if the specified matrix is symmetric
     * 
     * NOTE: squareness is not checked by this function
     *
     * @param y Matrix to test.
     * @return <code>true</code> if the matrix is symmetric.
     * @tparam T Type of scalar.
     */
    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_symmetric(const char* function,
                                const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                                const char* name,
                                T_result* result = 0,
                                const Policy& = Policy()) {
      typedef typename Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>::size_type size_type;
      size_type k = y.rows();
      if (k == 1)
        return true;
      
      for (size_type m = 0; m < k; ++m) {
        for (size_type n = m + 1; n < k; ++n) {
          if (fabs(y(m,n) - y(n,m)) > CONSTRAINT_TOLERANCE) {
            std::ostringstream message;
            message << name << " is not symmetric. " 
                    << name << "[" << m << "," << n << "] is %1%, but "
                    << name << "[" << n << "," << m << "] element is " << y(n,m);
            T_result tmp = policies::raise_domain_error<T_y>(function,
                                                             message.str().c_str(),
                                                             y(m,n), Policy());
            if (result != 0)
              *result = tmp;
            return false;
          }

        }
      }
      return true;
    }

    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    /**
     * Return <code>true</code> if the specified matrix is positive definite
     *
     * NOTE: symmetry is NOT checked by this function
     * 
     * @param y Matrix to test.
     * @return <code>true</code> if the matrix is positive definite.
     * @tparam T Type of scalar.
     */
    inline bool check_pos_definite(const char* function,
                                   const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                                   const char* name,
                                   T_result* result = 0,
                                   const Policy& = Policy()) {
      typedef typename Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>::size_type size_type;
      if (y.rows() == 1 && y(0,0) <= CONSTRAINT_TOLERANCE) {
        std::ostringstream message;
        message << name << " is not positive definite. " 
                << name << "(0,0) is %1%.";
        T_result tmp = policies::raise_domain_error<T_y>(function,
                                                         message.str().c_str(),
                                                         y(0,0), Policy());
        if (result != 0)
          *result = tmp;
        return false;
      }
      Eigen::LDLT< Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic> > cholesky = y.ldlt();
      if((cholesky.vectorD().array() <= CONSTRAINT_TOLERANCE).any())  {
        std::ostringstream message;
        message << name << " is not positive definite. " 
                << name << "(0,0) is %1%.";
        T_result tmp = policies::raise_domain_error<T_y>(function,
                                                         message.str().c_str(),
                                                         y(0,0), Policy());
        if (result != 0)
          *result = tmp;
        return false;
      }
      return true;
    }

    /**
     * Return <code>true</code> if the specified matrix is a valid
     * covariance matrix.  A valid covariance matrix must be square,
     * symmetric, and positive definite.
     *
     * @param y Matrix to test.
     * @return <code>true</code> if the matrix is a valid covariance matrix.
     * @tparam T Type of scalar.
     */
    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_cov_matrix(const char* function,
                                 const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                                 const char* name,
                                 T_result* result = 0,
                                 const Policy& = Policy()) {
      if (!check_size_match(function, y.rows(), y.cols(), result, Policy())) 
        return false;
      if (!check_positive(function, y.rows(), "rows", result, Policy()))
        return false;
      if (!check_symmetric(function, y, "y", result, Policy()))
        return false;
      if (!check_pos_definite(function, y, "y", result, Policy()))
        return false;
      return true;
    }


    /**
     * Return <code>true</code> if the specified matrix is a valid
     * correlation matrix.  A valid correlation matrix is symmetric,
     * has a unit diagonal (all 1 values), and has all values between
     * -1 and 1 (inclussive).  
     *
     * @param y Matrix to test.
     * @return <code>true</code> if the specified matrix is a valid
     * correlation matrix.
     * @tparam T Type of scalar.
     */
    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_corr_matrix(const char* function,
                                  const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                                  const char* name,
                                  T_result* result = 0,
                                  const Policy& = Policy()) {
      if (!check_size_match(function, y.rows(), y.cols(), result, Policy())) 
        return false;
      if (!check_positive(function, y.rows(), "rows", result, Policy()))
        return false;
      if (!check_symmetric(function, y, "y", result, Policy()))
        return false;
      for (typename Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>::size_type k = 0; k < y.rows(); ++k) {
        if (fabs(y(k,k) - 1.0) > CONSTRAINT_TOLERANCE) {
          std::ostringstream message;
          message << name << " is not a valid correlation matrix. " 
                  << name << "(" << k << "," << k << ") is %1%, but should be near 1.0";
          T_result tmp = policies::raise_domain_error<T_y>(function,
                                                           message.str().c_str(),
                                                           y(k,k), Policy());
          if (result != 0)
            *result = tmp;
          return false;
        }
      }
      if (!check_pos_definite(function, y, "y", result, Policy()))
        return false;
      return true;
    }
    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_not_nan(const char* function,
                              const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                              const char* name,
                              T_result* result = 0,
                              const Policy& = Policy()) {
      using stan::math::policies::raise_domain_error;
      for (int i = 0; i < y.rows(); i++) {
        if (boost::math::isnan(y[i])) {
          std::ostringstream message;
          message << name << "[" << i << "] is %1%, but must not be nan!";
          T_result tmp = raise_domain_error<T_result,T_y>(function,
                                                          message.str().c_str(),
                                                          y[i],
                                                          Policy());
          if (result != 0)
            *result = tmp;
          return false;
        }
      }
      return true;
    }


    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_not_nan(const char* function,
                              const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                              const char* name,
                              T_result* result = 0,
                              const Policy& = Policy()) {
      for (int i = 0; i < y.rows(); i++) {
        for (int j = 0; j < y.cols(); j++) {
          if (boost::math::isnan(y(i,j))) {
            std::ostringstream message;
            message << name << "[" << i << "," << j << "] is %1%, but must not be nan!";
            T_result tmp = policies::raise_domain_error<T_y>(function,
                                                             message.str().c_str(),
                                                             y(i,j), Policy());
            if (result != 0)
              *result = tmp;
            return false;
          }
        }
      }
      return true;
    }


    template <typename T_y, typename T_result = T_y, class Policy = default_policy>
    inline bool check_finite(const char* function,
                             const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                             const char* name,
                             T_result* result = 0,
                             const Policy& = Policy()) {
      using stan::math::policies::raise_domain_error;
      for (int i = 0; i < y.rows(); i++) {
        if (!boost::math::isfinite(y[i])) {
          std::ostringstream message;
          message << name << "[" << i << "] is %1%, but must be finite!";
          T_result tmp = raise_domain_error<T_result,T_y>(function,
                                                          message.str().c_str(),
                                                          y[i],
                                                          Policy());
          if (result != 0)
            *result = tmp;
          return false;
        }
      }
      return true;
    }

    /**
     * Return <code>true</code> if the specified matrix is a valid
     * covariance matrix.  A valid covariance matrix must be symmetric
     * and positive definite.
     *
     * @param y Matrix to test.
     * @param err_msg Output stream for error messages.
     * @return <code>true</code> if the matrix is a valid covariance matrix.
     * @tparam T Type of scalar.
     */
    template <typename T_covar, typename T_result = T_covar, class Policy = default_policy>
    inline bool check_cov_matrix(const char* function,
                                 const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                                 T_result* result = 0,
                                 const Policy& = Policy()) {
      if (!check_size_match(function, Sigma.rows(), Sigma.cols(), result, Policy())) 
        return false;
      if (!check_positive(function, Sigma.rows(), "rows", result, Policy()))
        return false;
      if (!check_symmetric(function, Sigma, "Sigma", result, Policy()))
        return false;
      return true;
    }

  }
}

#endif
