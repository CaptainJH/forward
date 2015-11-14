#ifndef _SINGLETON_H__
#define _SINGLETON_H__
#include <assert.h>

namespace forward 
{
// End SJS additions
    /** Template class for creating single-instance global classes.
    */
    template <typename T> class Singleton
    {
    private:
        /** @brief Explicit private copy constructor. This is a forbidden operation.*/
        Singleton(const Singleton<T> &);

        /** @brief Private operator= . This is a forbidden operation. */
        Singleton& operator=(const Singleton<T> &);
    
    protected:

        static T* msSingleton;

    public:
        Singleton( void )
        {
            assert( !msSingleton );
#if defined( _MSC_VER ) && _MSC_VER < 1200   
            int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
            msSingleton = (T*)((int)this + offset);
#else
            msSingleton = static_cast< T* >( this );
#endif
        }
        ~Singleton( void )
        {  assert( msSingleton );  msSingleton = 0;  }
        static T& getSingleton( void )
        {  assert( msSingleton );  return ( *msSingleton ); }
        static T* getSingletonPtr( void )
        { return msSingleton; }
    };
    /** @} */
    /** @} */

}

#endif