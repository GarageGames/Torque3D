#ifndef TORQUE_SIM_EVENT_DECLARATION_H
#define TORQUE_SIM_EVENT_DECLARATION_H

#ifndef _TSIGNAL_H_
   #include "core/util/tSignal.h"
#endif

template< typename OWNER, typename T >
struct EngineAppendOwner {};

template< typename OWNER, typename R >
struct EngineAppendOwner< OWNER, R() >
{
   typedef R( FunctionType )( typename OWNER* );   
};
template< typename OWNER, typename R, typename A >
struct EngineAppendOwner< OWNER, R( A ) >
{
   typedef R( FunctionType )( typename OWNER*, A );  
};
template< typename OWNER, typename R, typename A, typename B >
struct EngineAppendOwner< OWNER, R( A, B ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B );   
};
template< typename OWNER, typename R, typename A, typename B, typename C >
struct EngineAppendOwner< OWNER, R( A, B, C ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D >
struct EngineAppendOwner< OWNER, R( A, B, C, D ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F, G ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F, G );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F, G, H ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F, G, H );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F, G, H, I ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F, G, H, I );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F, G, H, I, J ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F, G, H, I, J );
};
template< typename OWNER, typename R, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K >
struct EngineAppendOwner< OWNER, R( A, B, C, D, E, F, G, H, I, J, K ) >
{
   typedef R( FunctionType )( typename OWNER*, A, B, C, D, E, F, G, H, I, J, K );
};

/// Declare a global signal and a script callback
/// When SIMDELEGATE are called with NAME##_callback, c++ signal are triggered first, then torque-script callback.
/// Torque::ISignal< void ARGS >> getISignal_##NAME()
/// @see IMPLEMENT_SIMSIGNAL
#define DECLARE_GLOBAL_SIMSIGNAL( NAME, ARGS )                                                                                                     \
   Torque::ISignal< void ARGS > getGlobal_ISignal_##NAME();                                                                                        \

/// Declare a global delegate and a script callback
/// When SIMDELEGATE are called with NAME##_callback, if a c++ delegate is set, torque-script callback are not executed.
/// void setDelegate_##NAME(const Delegate< returnType ARGS > &dlg);
/// Delegate< returnType ARGS > getDelegateCopy_##NAME() const;
//! @see IMPLEMENT_GLOBAL_SIMDELEGATE
#define DECLARE_GLOBAL_SIMDELEGATE( NAME, RET, ARGS )                                                                                              \
   void setDelegate_##NAME(const Delegate< RET ARGS > &dlg);                                                                                       \
   Delegate< RET ARGS > getDelegateCopy_##NAME();                                                                                                  \

/// Declare a member signal and a script callback
/// When SIMDELEGATE are called with NAME##_callback, c++ signal are triggered first, then torque-script callback.
/// This macro change the actual scope of class members to SCOPE
/// Torque::ISignal< void(ownerPtr + ARGS) >> getISignal_##NAME()
/// @see IMPLEMENT_SIMSIGNAL
#define DECLARE_SIMSIGNAL( SCOPE, NAME, ARGS )                                                                                                     \
SCOPE:                                                                                                                                             \
   virtual void NAME ## _callback ARGS ;                                                                                                           \
   Signal< EngineAppendOwner< privateThisClassType, void ARGS >::FunctionType > mSignal_##NAME;                                                    \
public:                                                                                                                                            \
   Torque::ISignal< EngineAppendOwner< privateThisClassType, void ARGS >::FunctionType > getISignal_##NAME() { return mSignal_##NAME; }            \
SCOPE:                                                                                                                                             \

/// Declare a member delegate and a script callback
/// When SIMDELEGATE are called with NAME##_callback, if a c++ delegate is set, torque-script callback are not executed.
/// This macro change the actual scope of class members to SCOPE
/// void setDelegate_##NAME(const Delegate< returnType (ownerPtr + ARGS) > &dlg);
/// Delegate< returnType (ownerPtr + ARGS) > getDelegateCopy_##NAME() const;
//! @see IMPLEMENT_SIMDELEGATE
#define DECLARE_SIMDELEGATE( SCOPE, returnType, NAME, ARGS )                                                                                       \
SCOPE:                                                                                                                                             \
   virtual returnType NAME ## _callback ARGS ;                                                                                                     \
   Delegate< EngineAppendOwner< privateThisClassType, returnType ARGS >::FunctionType > mDelegate_##NAME;                                          \
public:                                                                                                                                            \
   void setDelegate_##NAME(const Delegate< EngineAppendOwner< privateThisClassType, returnType ARGS >::FunctionType > &dlg)                        \
      { mDelegate_##NAME = dlg; }                                                                                                                  \
   Delegate< EngineAppendOwner< privateThisClassType, returnType ARGS >::FunctionType > getDelegateCopy_##NAME() const                             \
      { return mDelegate_##NAME; }                                                                                                                 \
SCOPE:                                                                                                                                             \
   
#endif //TORQUE_SIM_EVENT_DECLARATION_H