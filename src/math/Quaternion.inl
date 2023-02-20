
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real>::Quaternion( ) :
	w( 0 ),
	x( 0 ),
	y( 0 ),
	z( 0 )	
{
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real>::Quaternion( Real W, Real X, Real Y, Real Z ) :
	w( W ),
	x( X ),
	y( Y ),
	z( Z )
{
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real>::~Quaternion( )
{
}
//--------------------------------------------------------------------------------
template <typename Real>
Real forward::Quaternion<Real>::length() const
{
	return( sqrt( w*w + x*x + y*y + z*z ) );
}
//--------------------------------------------------------------------------------
template <typename Real>
Real forward::Quaternion<Real>::lengthSquared() const
{
	return( w*w + x*x + y*y + z*z );
}
//--------------------------------------------------------------------------------
template <typename Real>
Real forward::Quaternion<Real>::dot( const forward::Quaternion<Real>& a ) const
{
	return(	w*a.w + x*a.x + y*a.y + z*a.z );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::conjugate() const
{
	return( Quaternion<Real>( w, -x, -y, -z ) );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::inverse() const
{
	return( conjugate() / lengthSquared() );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator+( const forward::Quaternion<Real>& a ) const
{
	return( Quaternion<Real>( a.w+w, a.x+x, a.y+y, a.z+z ) );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator-( const forward::Quaternion<Real>& a ) const
{
	return( Quaternion<Real>( a.w-w, a.x-x, a.y-y, a.z-z ) );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator*( const forward::Quaternion<Real>& a ) const
{
	Quaternion q;

	q.w = a.w*w - a.x*x - a.y*y - a.z*z;
	q.x = a.w*x + a.x*w + a.y*z - a.z*y;
	q.y = a.w*y - a.x*z + a.y*w + a.z*x;
	q.z = a.w*z + a.x*y - a.y*x + a.z*w;

	return( q );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator/( const forward::Quaternion<Real>& a ) const
{
	return( *this * a.inverse() );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator*( const Real& real ) const
{
	return( Quaternion<Real>( w*real, x*real, y*real, z*real ) );
}
//--------------------------------------------------------------------------------
template <typename Real>
forward::Quaternion<Real> forward::Quaternion<Real>::operator/( const Real& real ) const
{
	assert( real != 0.0 );

	return( Quaternion<Real>( w/real, x/real, y/real, z/real ) );
}
//--------------------------------------------------------------------------------
