#include "LightSpacePerspectiveShadowMapMatrixCaculator.h"

#if ( LISPSM_ALGO == DIRECTIONAL_AND_SPOT )


// Adapted Modified version of LispSM authors implementation from 2006
// Nopt formula differs from the paper. I adopted original authors class to
// use with OSG



//we search the point in the LVS volume that is nearest to the camera
#include <limits.h>
static const float OSG_INFINITY = FLT_MAX;

namespace osgShadow {

class LispSM {
public:
    typedef std::vector<osg::Vec3d> Vertices;

    void setProjectionMatrix( const osg::Matrix & projectionMatrix )
        { _projectionMatrix = projectionMatrix; }

    void setViewMatrix( const osg::Matrix & viewMatrix )
        { _viewMatrix = viewMatrix; }

    void setHull( const ConvexPolyhedron & hull )
        { _hull = hull; }

    const ConvexPolyhedron & getHull( ) const
        { return _hull; }

    const osg::Matrix & getProjectionMatrix( void ) const
        { return _projectionMatrix; }

    const osg::Matrix & getViewMatrix( void ) const
        { return _viewMatrix; }

    bool getUseLiSPSM() const
        { return _useLiSPSM; }

    void setUseLiSPSM( bool use )
        { _useLiSPSM = use; }

    bool getUseFormula() const
        { return _useFormula; }

    void setUseFormula( bool use )
        { _useFormula = use; }

    bool getUseOldFormula() const
        { return _useOldFormula; }

    void setUseOldFormula( bool use )
        { _useOldFormula = use; }

    void setN(const double& n )
        { _N = n; }

    const double getN() const
        { return _N; }

    //for old LispSM formula from paper
    const double getNearDist() const
        { return _nearDist; }

    void setNearDist( const double & nearDist )
        { _nearDist = nearDist; }

    const double getFarDist() const
        { return _farDist; }

    void setFarDist( const double & farDist )
        { _farDist = farDist; }

    const osg::Vec3d & getEyeDir() const
        { return _eyeDir; }

    const osg::Vec3d & getLightDir() const
        { return _lightDir; }

    void setEyeDir( const osg::Vec3d eyeDir )
        { _eyeDir = eyeDir; }

    void setLightDir( const osg::Vec3d lightDir )
        { _lightDir = lightDir; }

protected:

    bool        _useLiSPSM;
    bool        _useFormula;
    bool        _useOldFormula;
    double      _N;
    double      _nearDist;
    double      _farDist;

    mutable osg::Vec3d  _E;
    osg::Vec3d  _eyeDir;
    osg::Vec3d  _lightDir;

    ConvexPolyhedron _hull;

    osg::Matrix _viewMatrix;
    osg::Matrix _projectionMatrix;

    double      getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;

    osg::Vec3d  getNearCameraPointE() const;

    osg::Vec3d  getZ0_ls
                    (const osg::Matrix& lightSpace, const osg::Vec3d& e, const double& b_lsZmax, const osg::Vec3d& eyeDir) const;

    double      calcNoptGeneral
                    (const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;

    double      calcNoptOld
                    ( const double gamma_ = 999) const;

    osg::Matrix getLispSmMtx
                    (const osg::Matrix& lightSpace) const;

    osg::Vec3d  getProjViewDir_ls
                    (const osg::Matrix& lightSpace) const;

    void        updateLightMtx
                    (osg::Matrix& lightView, osg::Matrix& lightProj, const std::vector<osg::Vec3d>& B) const;

public:
    LispSM( ) : _useLiSPSM( true ), _useFormula( true ), _useOldFormula( false ), _N( 1 ), _nearDist( 1 ), _farDist( 10 ) { }

    virtual void updateLightMtx( osg::Matrix& lightView, osg::Matrix& lightProj ) const;
};

}

osg::Vec3d LispSM::getNearCameraPointE( ) const
{
    const osg::Matrix& eyeView = getViewMatrix();

    ConvexPolyhedron::Vertices LVS;
    _hull.getPoints( LVS );

    //the LVS volume is always in front of the camera
    //the camera points along the neg z axis.
    //-> so the nearest point is the maximum

    unsigned max = 0;
    for(unsigned i = 0; i < LVS.size(); i++) {

        LVS[i] = LVS[i] * eyeView;

        if( LVS[max].z() < LVS[i].z() ) {
            max = i;
        }
    }
    //transform back to world space
    return LVS[max] * osg::Matrix::inverse( eyeView );
}

//z0 is the point that lies on the plane A parallel to the near plane through e
//and on the near plane of the C frustum (the plane z = bZmax) and on the line x = e.x
osg::Vec3d LispSM::getZ0_ls
    (const osg::Matrix& lightSpace, const osg::Vec3d& e, const double& b_lsZmax, const osg::Vec3d& eyeDir) const
{
    //to calculate the parallel plane to the near plane through e we
    //calculate the plane A with the three points
    osg::Plane A(eyeDir,e);
    //to transform plane A into lightSpace
    A.transform( lightSpace );
    //transform to light space
    const osg::Vec3d e_ls = e * lightSpace;

    //z_0 has the x coordinate of e, the z coord of B_lsZmax
    //and the y coord of the plane A and plane (z==B_lsZmax) intersection
#if 1
    osg::Vec3d v = osg::Vec3d(e_ls.x(),0,b_lsZmax);

    // x & z are given. We compute y from equations:
    // A.distance( x,y,z ) == 0
    // A.distance( x,y,z ) == A.distance( x,0,z ) + A.normal.y * y
    // hence A.distance( x,0,z ) == -A.normal.y * y

    v.y() = -A.distance( v ) / A.getNormal().y();
#else
    //get the parameters of A from the plane equation n dot d = 0
    const double d = A.asVec4()[3];
    const osg::Vec3d n = A.getNormal();
    osg::Vec3d v(e_ls.x(),(-d-n.z()*b_lsZmax-n.x()*e_ls.x())/n.y(),b_lsZmax);
#endif

    return v;

}

double LispSM::calcNoptGeneral(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const
{
    const osg::Matrix& eyeView = getViewMatrix();
    const osg::Matrix invLightSpace = osg::Matrix::inverse( lightSpace );

    const osg::Vec3d z0_ls = getZ0_ls(lightSpace, _E,B_ls.zMax(),getEyeDir());
    const osg::Vec3d z1_ls = osg::Vec3d(z0_ls.x(),z0_ls.y(),B_ls.zMin());

    //to world
    const osg::Vec4d z0_ws = osg::Vec4d( z0_ls, 1 ) * invLightSpace;
    const osg::Vec4d z1_ws = osg::Vec4d( z1_ls, 1 ) * invLightSpace;

    //to eye
    const osg::Vec4d z0_cs = z0_ws * eyeView;
    const osg::Vec4d z1_cs = z1_ws * eyeView;

    double z0 = -z0_cs.z() / z0_cs.w();
    double z1 = -z1_cs.z() / z1_cs.w();

    if( z1 / z0 <= 1.0 ) {

        // solve camera pos singularity in light space problem brutally:
        // if extreme points of B projected to Light space extend beyond
        // camera frustum simply use B extents in camera frustum

        // Its not optimal selection but ceratainly better than negative N
        osg::BoundingBox bb = _hull.computeBoundingBox( eyeView );
        z0 = -bb.zMax();
        if( z0 <= 0 )
            z0 = 0.1;

        z1 = -bb.zMin();
        if( z1 <= z0 )
            z1 = z0 + 0.1;
    }

    const double d = osg::absolute(B_ls.zMax()-B_ls.zMin());

    double N = d/( sqrt( z1 / z0 ) - 1.0 );
#if PRINT_COMPUTED_N_OPT
    std::cout
       << " N=" << std::setw(8) << N
       << " n=" << std::setw(8) << z0
       << " f=" << std::setw(8) << z1
       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
       << std::flush;
#endif
    return N;
}

double LispSM::calcNoptOld( const double gamma_ ) const
{
    const double& n = getNearDist();
    const double& f = getFarDist();
    const double d = abs(f-n);
    double sinGamma(0);
    if(999 == gamma_) {
        double dot = getEyeDir() * getLightDir();
        sinGamma = sqrt( 1.0 - dot * dot );
    }
    else {
        sinGamma = sin(gamma_);
    }

    double N = (n+sqrt(n*(n+d*sinGamma)))/sinGamma;
#if PRINT_COMPUTED_N_OPT
    std::cout
       << " N=" << std::setw(8) << N
       << " n=" << std::setw(8) << n
       << " f=" << std::setw(8) << f
       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
       << std::flush;
#endif
    return N;
}

double LispSM::getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const
{
    if( getUseFormula()) {
        if( getUseOldFormula() )
            return calcNoptOld();
        else
            return calcNoptGeneral(lightSpace,B_ls);
    }
    else {
        return getN();
    }
}
//this is the algorithm discussed in the article
osg::Matrix LispSM::getLispSmMtx( const osg::Matrix& lightSpace ) const
{
    const osg::BoundingBox B_ls = _hull.computeBoundingBox( lightSpace );

    const double n = getN(lightSpace,B_ls);

    //get the coordinates of the near camera point in light space
    const osg::Vec3d e_ls = _E * lightSpace;
    //c start has the x and y coordinate of e, the z coord of B.min()
    const osg::Vec3d Cstart_lp(e_ls.x(),e_ls.y(),B_ls.zMax());

    if( n >= OSG_INFINITY ) {
        //if n is inf. than we should do uniform shadow mapping
        return osg::Matrix::identity();
    }
    //calc C the projection center
    //new projection center C, n behind the near plane of P
    //we work along a negative axis so we transform +n*<the positive axis> == -n*<neg axis>
    const osg::Vec3d C( Cstart_lp + osg::Vec3d(0,0,1) * n );
    //construct a translation that moves to the projection center
    const osg::Matrix projectionCenter = osg::Matrix::translate( -C );

    //calc d the perspective transform depth or light space y extents
    const double d = osg::absolute(B_ls.zMax()-B_ls.zMin());

    //the lispsm perspective transformation

    //here done with a standard frustum call that maps P onto the unit cube with
    //corner points [-1,-1,-1] and [1,1,1].
    //in directX you can use the same mapping and do a mapping to the directX post-perspective cube
    //with corner points [-1,-1,0] and [1,1,1] as the final step after all the shadow mapping.
    osg::Matrix P = osg::Matrix::frustum( -1.0,1.0,-1.0,1.0, n, n+d );

    //invert the transform from right handed into left handed coordinate system for the ndc
    //done by the openGL style frustumGL call
    //so we stay in a right handed system
    P = P * osg::Matrix::scale( 1.0,1.0,-1.0 );
    //return the lispsm frustum with the projection center
    return projectionCenter * P;
}

osg::Vec3d LispSM::getProjViewDir_ls(const osg::Matrix& lightSpace ) const {
    //get the point in the LVS volume that is nearest to the camera
    const osg::Vec3d e = _E;
    //construct edge to transform into light-space
    const osg::Vec3d b = e+getEyeDir();
    //transform to light-space
    osg::Vec4d e_lp = osg::Vec4d( e, 1.0 ) * lightSpace;
    osg::Vec4d b_lp = osg::Vec4d( b, 1.0 ) * lightSpace;

    if( e_lp[3] <= 0 )
    {
        e_lp[3] = e_lp[3];
    }

    if( b_lp[3] <= 0 )
    {
        osg::Vec4d v = (e_lp - b_lp)/(e_lp[3]-b_lp[3]);

        v = ( e_lp + v  ) * 0.5;

        b_lp = v;
    }

    osg::Vec3d projDir( osg::Vec3( b_lp[0], b_lp[1], b_lp[2] ) / b_lp[3] -
                        osg::Vec3( e_lp[0], e_lp[1], e_lp[2] ) / e_lp[3] );

    projDir.normalize();

    //project the view direction into the shadow map plane
    projDir.y() = 0.0;
    return projDir;
}

void LispSM::updateLightMtx
    ( osg::Matrix& lightView, osg::Matrix& lightProj ) const
{
    //calculate standard light space for spot or directional lights
    //this routine returns two matrices:
    //lightview contains the rotated translated frame
    //lightproj contains in the case of a spot light the spot light perspective transformation
    //in the case of a directional light a identity matrix
    // calcLightSpace(lightView,lightProj);

    if( _hull._faces.empty() ) {
        //debug() << "empty intersection body -> completely inside shadow\n";//debug output
        return;
    }

    _E = getNearCameraPointE();

    lightProj = lightProj * osg::Matrix::scale( 1, 1, -1 );

    //coordinate system change for calculations in the article
    osg::Matrix switchToArticle = osg::Matrix::identity();
    switchToArticle(1,1) = 0.0;
    switchToArticle(1,2) =-1.0; // y -> -z
    switchToArticle(2,1) = 1.0; // z -> y
    switchToArticle(2,2) = 0.0;
    //switch to the lightspace used in the article
    lightProj = lightProj * switchToArticle;

    osg::Matrix L = lightView * lightProj;

    osg::Vec3d projViewDir = getProjViewDir_ls(L);

    if( getUseLiSPSM() /* && projViewDir.z() < 0*/ ) {
        //do Light Space Perspective shadow mapping
        //rotate the lightspace so that the proj light view always points upwards
        //calculate a frame matrix that uses the projViewDir[light-space] as up vector
        //look(from position, into the direction of the projected direction, with unchanged up-vector)
        lightProj = lightProj *
            osg::Matrix::lookAt( osg::Vec3d(0,0,0), projViewDir, osg::Vec3d(0,1,0) );

        osg::Matrix lispsm = getLispSmMtx( lightView * lightProj );
        lightProj = lightProj * lispsm;
    }

    const osg::Matrix PL = lightView * lightProj;

    osg::BoundingBox bb = _hull.computeBoundingBox( PL );

    osg::Matrix fitToUnitFrustum;
    fitToUnitFrustum.makeOrtho( bb._min[0],  bb._max[0],
                                bb._min[1],  bb._max[1],
                               -bb._max[2], -bb._min[2] );

    //map to unit cube
    lightProj = lightProj * fitToUnitFrustum;

    //coordinate system change for calculations in the article
    osg::Matrix switchToGL = osg::Matrix::identity();
    switchToGL(1,1) =  0.0;
    switchToGL(1,2) =  1.0; // y -> z
    switchToGL(2,1) = -1.0; // z -> -y
    switchToGL(2,2) =  0.0;

    //back to open gl coordinate system y <-> z
    lightProj = lightProj * switchToGL;
    //transform from right handed system into left handed ndc
    lightProj = lightProj * osg::Matrix::scale(1.0,1.0,-1.0);
}

void LightSpacePerspectiveShadowMapAlgorithm::operator()
    ( const osgShadow::ConvexPolyhedron* hullShadowedView,
      const osg::Camera* cameraMain,
      osg::Camera* cameraShadow ) const
{
    lispsm->setHull( *hullShadowedView );
    lispsm->setViewMatrix( cameraMain->getViewMatrix() );
    lispsm->setProjectionMatrix( cameraMain->getViewMatrix() );

#if 1
    osg::Vec3d lightDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraShadow->getViewMatrix() ) );
    osg::Vec3d eyeDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraMain->getViewMatrix() ) );

#else
    osg::Vec3d lightDir = osg::Matrix::transform3x3( cameraShadow->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );
    osg::Vec3d eyeDir = osg::Matrix::transform3x3( cameraMain->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );
#endif

    lightDir.normalize();
    eyeDir.normalize();

    lispsm->setLightDir(lightDir);


    osg::Matrix &proj = cameraShadow->getProjectionMatrix();
    double l,r,b,t,n,f;
    if( proj.getOrtho( l,r,b,t,n,f ) )
    {
        osg::Vec3d camPosInLightSpace =
            osg::Vec3d( 0, 0, 0 ) *
            osg::Matrix::inverse( cameraMain->getViewMatrix() ) *
            cameraShadow->getViewMatrix() *
            cameraShadow->getProjectionMatrix();
    }

    eyeDir.normalize();

    lispsm->setEyeDir( eyeDir );

    osg::BoundingBox bb =
        hullShadowedView->computeBoundingBox( cameraMain->getViewMatrix() );

    lispsm->setNearDist( -bb.zMax() );
    lispsm->setFarDist( -bb.zMin() );

    lispsm->updateLightMtx
        ( cameraShadow->getViewMatrix(), cameraShadow->getProjectionMatrix() );
}

LightSpacePerspectiveShadowMapAlgorithm::LightSpacePerspectiveShadowMapAlgorithm()
{
    lispsm = new LispSM;
}

LightSpacePerspectiveShadowMapAlgorithm::~LightSpacePerspectiveShadowMapAlgorithm()
{
    delete lispsm;
}


#endif