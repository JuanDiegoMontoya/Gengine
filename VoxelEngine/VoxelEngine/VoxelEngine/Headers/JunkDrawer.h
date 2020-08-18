#ifndef JunkDrawer_Guard
#define JunkDrawer_Guard

#include <boost/preprocessor/cat.hpp>


#define DoOnce static bool BOOST_PP_CAT(doOnce_, __LINE__) = true; if (BOOST_PP_CAT(doOnce_, __LINE__) && !((BOOST_PP_CAT(doOnce_, __LINE__) = false)))


#endif // !JunkDrawer_Guard