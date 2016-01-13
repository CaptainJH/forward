#include "LightSpacePerspectiveShadowMapMatrixCaculator.h"
#include "Plane3f.h"
#include <limits.h>

using namespace forward;

Matrix4f ExpandZ(
	1, 0,  0, 0,
	0, 1,  0, 0,
	0, 0,  2, 0,
	0, 0, -1, 1);

Matrix4f ShrinkZ(
	1, 0, 0,   0,
	0, 1, 0,   0,
	0, 0, 0.5, 0,
	0, 0, 0.5, 1
	);

Matrix4f NormalToLightSpace(
	1, 0,  0, 0,      // x
	0, 0,  1, 0,      // y
	0, 1,  0, 0,      // z
	0, 0,  0, 1); // w

Matrix4f LightSpaceToNormal(
	1, 0, 0, 0,      // x
	0, 0, 1, 0,      // y
	0, 1, 0, 0,      // z
	0, 0, 0, 1); // w

Vector3f operator*(const Vector3f& vec3, const Matrix4f& mat)
{
	Vector4f vec4(vec3, 1.0f);
	vec4 = mat * vec4;
	vec4 /= vec4.w;

	return vec4.xyz();
}

LightSpacePerspectiveShadowMapMatrixCaculator::LightSpacePerspectiveShadowMapMatrixCaculator(const Camera& lc, const Camera& vc)
	: m_lightCamera(lc)
	, m_viewCamera(vc)
{
}

void LightSpacePerspectiveShadowMapMatrixCaculator::addSceneObjects(const GeometryDX11* obj)
{
	if (std::find(m_sceneObjects.begin(), m_sceneObjects.end(), obj) == m_sceneObjects.end())
		m_sceneObjects.push_back(obj);
}

void LightSpacePerspectiveShadowMapMatrixCaculator::linkWorldMatrixOfObject(const GeometryDX11* obj, const Matrix4f* worldMat)
{
	if (std::find(m_sceneObjects.begin(), m_sceneObjects.end(), obj) == m_sceneObjects.end())
		return;

	if (m_matrixLinks.find(obj) == m_matrixLinks.end())
		m_matrixLinks[obj] = worldMat;
}

// Adapted Modified version of LispSM authors implementation from 2006
// Nopt formula differs from the paper. I adopted original authors class to
// use with OSG



//we search the point in the LVS volume that is nearest to the camera

//static const float OSG_INFINITY = FLT_MAX;

//namespace osgShadow {
//
//class LispSM {
//public:
//    typedef std::vector<osg::Vec3d> Vertices;
//
//    void setProjectionMatrix( const osg::Matrix & projectionMatrix )
//        { _projectionMatrix = projectionMatrix; }
//
//    void setViewMatrix( const osg::Matrix & viewMatrix )
//        { _viewMatrix = viewMatrix; }
//
//    void setHull( const ConvexPolyhedron & hull )
//        { _hull = hull; }
//
//    const ConvexPolyhedron & getHull( ) const
//        { return _hull; }
//
//    const osg::Matrix & getProjectionMatrix( void ) const
//        { return _projectionMatrix; }
//
//    const osg::Matrix & getViewMatrix( void ) const
//        { return _viewMatrix; }
//
//    bool getUseLiSPSM() const
//        { return _useLiSPSM; }
//
//    void setUseLiSPSM( bool use )
//        { _useLiSPSM = use; }
//
//    bool getUseFormula() const
//        { return _useFormula; }
//
//    void setUseFormula( bool use )
//        { _useFormula = use; }
//
//    bool getUseOldFormula() const
//        { return _useOldFormula; }
//
//    void setUseOldFormula( bool use )
//        { _useOldFormula = use; }
//
//    void setN(const double& n )
//        { _N = n; }
//
//    const double getN() const
//        { return _N; }
//
//    //for old LispSM formula from paper
//    const double getNearDist() const
//        { return _nearDist; }
//
//    void setNearDist( const double & nearDist )
//        { _nearDist = nearDist; }
//
//    const double getFarDist() const
//        { return _farDist; }
//
//    void setFarDist( const double & farDist )
//        { _farDist = farDist; }
//
//    const osg::Vec3d & getEyeDir() const
//        { return _eyeDir; }
//
//    const osg::Vec3d & getLightDir() const
//        { return _lightDir; }
//
//    void setEyeDir( const osg::Vec3d eyeDir )
//        { _eyeDir = eyeDir; }
//
//    void setLightDir( const osg::Vec3d lightDir )
//        { _lightDir = lightDir; }
//
//protected:
//
//    bool        _useLiSPSM;
//    bool        _useFormula;
//    bool        _useOldFormula;
//    double      _N;
//    double      _nearDist;
//    double      _farDist;
//
//    mutable osg::Vec3d  _E;
//    osg::Vec3d  _eyeDir;
//    osg::Vec3d  _lightDir;
//
//    ConvexPolyhedron _hull;
//
//    osg::Matrix _viewMatrix;
//    osg::Matrix _projectionMatrix;
//
//    double      getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;
//
//    osg::Vec3d  getNearCameraPointE() const;
//
//    osg::Vec3d  getZ0_ls
//                    (const osg::Matrix& lightSpace, const osg::Vec3d& e, const double& b_lsZmax, const osg::Vec3d& eyeDir) const;
//
//    double      calcNoptGeneral
//                    (const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const;
//
//    double      calcNoptOld
//                    ( const double gamma_ = 999) const;
//
//    osg::Matrix getLispSmMtx
//                    (const osg::Matrix& lightSpace) const;
//
//    osg::Vec3d  getProjViewDir_ls
//                    (const osg::Matrix& lightSpace) const;
//
//    void        updateLightMtx
//                    (osg::Matrix& lightView, osg::Matrix& lightProj, const std::vector<osg::Vec3d>& B) const;
//
//public:
//    LispSM( ) : _useLiSPSM( true ), _useFormula( true ), _useOldFormula( false ), _N( 1 ), _nearDist( 1 ), _farDist( 10 ) { }
//
//    virtual void updateLightMtx( osg::Matrix& lightView, osg::Matrix& lightProj ) const;
//};
//
//}

Vector3f LightSpacePerspectiveShadowMapMatrixCaculator::getNearCameraPointE( ) const
{
	const Matrix4f& eyeView = m_viewCamera.getViewMatrix();
	Vector3f nearestV(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3f ret;

	for (auto obj : m_sceneObjects)
	{
		Matrix4f worldMatrix = Matrix4f::Identity();
		auto itMat = m_matrixLinks.find(obj);
		if ( itMat != m_matrixLinks.end())
			worldMatrix = *itMat->second;

		for (auto v : obj->m_vElements)
		{
			if (v->m_SemanticName == VertexElementDX11::PositionSemantic)
			{
				for (auto i = 0; i < v->Count(); ++i)
				{
					const Vector4f& pos = v->Tuple() == 3 ? Vector4f(*v->Get3f(i), 1.0f) : *v->Get4f(i);
					Vector4f posL = worldMatrix * eyeView * pos;
					posL /= posL.w;

					if (posL.z < nearestV.z)
					{
						nearestV = posL.xyz();
						ret = pos.xyz();
					}
				}
			}
		}
	}

	return ret;
}

//z0 is the point that lies on the plane A parallel to the near plane through e
//and on the near plane of the C frustum (the plane z = bZmax) and on the line x = e.x
Vector3f LightSpacePerspectiveShadowMapMatrixCaculator::getZ0_ls(const Matrix4f& lightSpace, const Vector3f& e, const f32& b_lsZmax, const Vector3f& eyeDir) const
{
	const Vector3f e_ls = e * lightSpace;
	const Vector3f p_ls = (e + eyeDir) * lightSpace;

    //to calculate the parallel plane to the near plane through e we
    //calculate the plane A with the three points
	const Vector3f N = Vector3f::Normalize(p_ls - e_ls);
    Plane3f A(N, e_ls);

    //z_0 has the x coordinate of e, the z coord of B_lsZmax
    //and the y coord of the plane A and plane (z==B_lsZmax) intersection
    Vector3f v = Vector3f(e_ls.x, 0, b_lsZmax);

    // x & z are given. We compute y from equations:
    // A.distance( x,y,z ) == 0
    // A.distance( x,y,z ) == A.distance( x,0,z ) + A.normal.y * y
    // hence A.distance( x,0,z ) == -A.normal.y * y

    v.y = -A.DistanceToPoint(v) / N.y;

    return v;

}

f32 LightSpacePerspectiveShadowMapMatrixCaculator::calcNoptGeneral(const Matrix4f& lightSpace, const std::pair<Vector3f, Vector3f>& B_ls) const
{
    const Matrix4f& eyeView = m_viewCamera.getViewMatrix();
	const Matrix4f invLightSpace = lightSpace.Inverse();

    const Vector3f z0_ls = getZ0_ls(lightSpace, _E, B_ls.first.z, m_viewCamera.getWorldLookingDir());
    const Vector3f z1_ls = Vector3f(z0_ls.x, z0_ls.y, B_ls.second.z);

    //to world
    const Vector4f z0_ws = invLightSpace * Vector4f(z0_ls, 1);
    const Vector4f z1_ws = invLightSpace * Vector4f(z1_ls, 1);

    //to eye
    const Vector4f z0_cs = eyeView * z0_ws;
    const Vector4f z1_cs = eyeView * z1_ws;

    f32 z0 = -z0_cs.z / z0_cs.w;
    f32 z1 = -z1_cs.z / z1_cs.w;

    if( z1 / z0 <= 1.0f ) 
	{
        // solve camera pos singularity in light space problem brutally:
        // if extreme points of B projected to Light space extend beyond
        // camera frustum simply use B extents in camera frustum

        // Its not optimal selection but ceratainly better than negative N
        auto bb = computeBoundingBox( eyeView );
        z0 = -bb.second.z;
        if( z0 <= 0 )
            z0 = 0.1f;

        z1 = -bb.first.z;
        if( z1 <= z0 )
            z1 = z0 + 0.1f;
    }

    const f32 d = fabs(B_ls.second.z - B_ls.first.z);

    f32 N = d/( sqrt( z1 / z0 ) - 1.0f );

    return N;
}

//double LispSM::calcNoptOld( const double gamma_ ) const
//{
//    const double& n = getNearDist();
//    const double& f = getFarDist();
//    const double d = abs(f-n);
//    double sinGamma(0);
//    if(999 == gamma_) {
//        double dot = getEyeDir() * getLightDir();
//        sinGamma = sqrt( 1.0 - dot * dot );
//    }
//    else {
//        sinGamma = sin(gamma_);
//    }
//
//    double N = (n+sqrt(n*(n+d*sinGamma)))/sinGamma;
//#if PRINT_COMPUTED_N_OPT
//    std::cout
//       << " N=" << std::setw(8) << N
//       << " n=" << std::setw(8) << n
//       << " f=" << std::setw(8) << f
//       << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"
//       << std::flush;
//#endif
//    return N;
//}
//
//double LispSM::getN(const osg::Matrix lightSpace, const osg::BoundingBox& B_ls) const
//{
//    if( getUseFormula()) {
//        if( getUseOldFormula() )
//            return calcNoptOld();
//        else
//            return calcNoptGeneral(lightSpace,B_ls);
//    }
//    else {
//        return getN();
//    }
//}

std::pair<Vector3f, Vector3f> LightSpacePerspectiveShadowMapMatrixCaculator::computeBoundingBox(const Matrix4f& mat) const
{
	Vector3f minV(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3f maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (auto obj : m_sceneObjects)
	{
		Matrix4f worldMatrix = Matrix4f::Identity();
		auto itMat = m_matrixLinks.find(obj);
		if (itMat != m_matrixLinks.end())
			worldMatrix = *itMat->second;

		for (auto v : obj->m_vElements)
		{
			if (v->m_SemanticName == VertexElementDX11::PositionSemantic)
			{
				for (auto i = 0; i < v->Count(); ++i)
				{
					const Vector4f& pos = v->Tuple() == 3 ? Vector4f(*v->Get3f(i), 1.0f) : *v->Get4f(i);
					Vector3f pos3 = pos.xyz() * worldMatrix * mat;
					if (pos3.x < minV.x)
						minV.x = pos3.x;
					else if (pos3.x > maxV.x)
						maxV.x = pos3.x;

					if (pos3.y < minV.y)
						minV.y = pos3.y;
					else if (pos3.y > maxV.y)
						maxV.y = pos3.y;

					if (pos3.z < minV.z)
						minV.z = pos3.z;
					else if (pos3.z > maxV.z)
						maxV.z = pos3.z;
				}
			}
		}
	}

	const f32 safeBias = 0.1f;
	minV.z -= safeBias;
	maxV.z += safeBias;
	return std::make_pair(minV, maxV);
}

//this is the algorithm discussed in the article
Matrix4f LightSpacePerspectiveShadowMapMatrixCaculator::getLispSmMtx( const Matrix4f& lightSpace ) const
{
    //const osg::BoundingBox B_ls = _hull.computeBoundingBox( lightSpace );
	auto B_ls = computeBoundingBox(lightSpace);

    const f32 n = calcNoptGeneral(lightSpace, B_ls);
	//const f32 n = B_ls.first.z - 0.1f;
	//const f32 l = B_ls.second.z - B_ls.first.z;
	const f32 zRange = B_ls.second.z - B_ls.first.z;

	const Vector3f eye_ws = m_viewCamera.getWorldEyePos();
	const f32 projLength = _E.Dot(m_viewCamera.getWorldLookingDir());
	const Vector3f center = eye_ws + m_viewCamera.getWorldLookingDir() * projLength;

    //get the coordinates of the near camera point in light space
    const Vector3f e_ls = center * lightSpace;
    //c start has the x and y coordinate of e, the z coord of B.min()
    const Vector3f Cstart_lp(e_ls.x, e_ls.y, B_ls.first.z);

    //if( n >= OSG_INFINITY ) {
    //    //if n is inf. than we should do uniform shadow mapping
    //    return osg::Matrix::identity();
    //}

    //calc C the projection center
    //new projection center C, n behind the near plane of P
    //we work along a negative axis so we transform +n*<the positive axis> == -n*<neg axis>
    const Vector3f C( Cstart_lp - Vector3f(0.0f, 0.0f, 1.0f) * n );
    //construct a translation that moves to the projection center
	const Matrix4f projectionCenter = Matrix4f::TranslationMatrix(-C.x, -C.y, -C.z);

    //calc d the perspective transform depth or light space y extents
    const f32 d = fabs(B_ls.second.z - B_ls.first.z);

    //the lispsm perspective transformation

    //here done with a standard frustum call that maps P onto the unit cube with
    //corner points [-1,-1,-1] and [1,1,1].
    //in directX you can use the same mapping and do a mapping to the directX post-perspective cube
    //with corner points [-1,-1,0] and [1,1,1] as the final step after all the shadow mapping.
    Matrix4f P = Matrix4f::PerspectiveFrustumLHMatrix( -1.0, 1.0, -1.0, 1.0, n, n + d );

    //invert the transform from right handed into left handed coordinate system for the ndc
    //done by the openGL style frustumGL call
    //so we stay in a right handed system
    //P = P * osg::Matrix::scale( 1.0,1.0,-1.0 );
    //return the lispsm frustum with the projection center
    return projectionCenter * P * ExpandZ;
}

Vector3f LightSpacePerspectiveShadowMapMatrixCaculator::getProjViewDir_ls(const Matrix4f& lightSpace) const
{
    //get the point in the LVS volume that is nearest to the camera
    const Vector3f e = _E;
    //construct edge to transform into light-space
	const Vector3f b = e + m_viewCamera.getWorldLookingDir();
    //transform to light-space
    Vector4f e_lp = lightSpace * Vector4f( e, 1.0 );
    Vector4f b_lp = lightSpace * Vector4f( b, 1.0 );

    if( e_lp[3] <= 0 )
    {
        e_lp[3] = e_lp[3];
    }

    if( b_lp[3] <= 0 )
    {
        Vector4f v = (e_lp - b_lp)/(e_lp[3]-b_lp[3]);

        v = ( e_lp + v  ) * 0.5;

        b_lp = v;
    }

    Vector3f projDir( Vector3f( b_lp[0], b_lp[1], b_lp[2] ) / b_lp[3] -
                        Vector3f( e_lp[0], e_lp[1], e_lp[2] ) / e_lp[3] );

	projDir.Normalize();

    //project the view direction into the shadow map plane
    projDir.y = 0.0;
    return projDir;
}

Matrix4f LightSpacePerspectiveShadowMapMatrixCaculator::update()
{
	Matrix4f ToGL = Matrix4f::ScaleMatrix(Vector3f(1.0f, 1.0f, -1.0f));
	Matrix4f lightView = m_lightCamera.getViewMatrix();
	Matrix4f lightProj = m_lightCamera.getProjectionMatrix() * ExpandZ * NormalToLightSpace;

    //calculate standard light space for spot or directional lights
    //this routine returns two matrices:
    //lightview contains the rotated translated frame
    //lightproj contains in the case of a spot light the spot light perspective transformation
    //in the case of a directional light a identity matrix
    // calcLightSpace(lightView,lightProj);

    if( m_sceneObjects.empty() ) 
	{
		return Matrix4f::Identity();
    }

    _E = getNearCameraPointE();

    Matrix4f L = lightView * lightProj;
    Vector3f projViewDir = getProjViewDir_ls(L);

	MatrixTester(L);

	{
        //do Light Space Perspective shadow mapping
        //rotate the lightspace so that the proj light view always points upwards
        //calculate a frame matrix that uses the projViewDir[light-space] as up vector
        //look(from position, into the direction of the projected direction, with unchanged up-vector)
        lightProj = lightProj *
            Matrix4f::LookAtLHMatrix( Vector3f(0.0f, 0.0f, 0.0f), projViewDir, Vector3f(0.0f, 1.0f, 0.0f) );

        Matrix4f lispsm = getLispSmMtx( lightView * lightProj );
        lightProj = lightProj * lispsm;
    }

    const Matrix4f PL = lightView * lightProj;
	MatrixTester(PL);

    auto bb = computeBoundingBox( PL );

	const f32 SafeBias = 0.0f;
	Matrix4f fitToUnitFrustum = Matrix4f::OrthographicLHMatrix(bb.first.x, bb.second.x, bb.first.y, bb.second.y, bb.first.z - SafeBias, bb.second.z + SafeBias) * ExpandZ;

    //map to unit cube
    lightProj = lightProj * fitToUnitFrustum;

    //coordinate system change for calculations in the article
    //osg::Matrix switchToGL = osg::Matrix::identity();
    //switchToGL(1,1) =  0.0;
    //switchToGL(1,2) =  1.0; // y -> z
    //switchToGL(2,1) = -1.0; // z -> -y
    //switchToGL(2,2) =  0.0;

    //back to open gl coordinate system y <-> z
    //lightProj = lightProj * switchToGL;
    //transform from right handed system into left handed ndc
    //lightProj = lightProj * osg::Matrix::scale(1.0,1.0,-1.0);

	m_finalMat = lightView * lightProj * LightSpaceToNormal * ShrinkZ;
	MatrixTester(m_finalMat);
	return m_finalMat;
}

//void LightSpacePerspectiveShadowMapAlgorithm::operator()
//    ( const osgShadow::ConvexPolyhedron* hullShadowedView,
//      const osg::Camera* cameraMain,
//      osg::Camera* cameraShadow ) const
//{
//    lispsm->setHull( *hullShadowedView );
//    lispsm->setViewMatrix( cameraMain->getViewMatrix() );
//    lispsm->setProjectionMatrix( cameraMain->getViewMatrix() );
//
//#if 1
//    osg::Vec3d lightDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraShadow->getViewMatrix() ) );
//    osg::Vec3d eyeDir = osg::Matrix::transform3x3( osg::Vec3d( 0, 0, -1 ), osg::Matrix::inverse( cameraMain->getViewMatrix() ) );
//
//#else
//    osg::Vec3d lightDir = osg::Matrix::transform3x3( cameraShadow->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );
//    osg::Vec3d eyeDir = osg::Matrix::transform3x3( cameraMain->getViewMatrix(), osg::Vec3d( 0.0, 0.0, -1.0 ) );
//#endif
//
//    lightDir.normalize();
//    eyeDir.normalize();
//
//    lispsm->setLightDir(lightDir);
//
//    eyeDir.normalize();
//
//    lispsm->setEyeDir( eyeDir );
//
//    osg::BoundingBox bb =
//        hullShadowedView->computeBoundingBox( cameraMain->getViewMatrix() );
//
//    lispsm->setNearDist( -bb.zMax() );
//    lispsm->setFarDist( -bb.zMin() );
//
//    lispsm->updateLightMtx
//        ( cameraShadow->getViewMatrix(), cameraShadow->getProjectionMatrix() );
//}

void LightSpacePerspectiveShadowMapMatrixCaculator::MatrixTester(const Matrix4f& mat)
{
	m_tempVs.clear();
	m_tempV4.clear();

	for (auto obj : m_sceneObjects)
	{
		for (auto v : obj->m_vElements)
		{
			if (v->m_SemanticName == VertexElementDX11::PositionSemantic)
			{
				for (auto i = 0; i < v->Count(); ++i)
				{
					const Vector4f& pos = v->Tuple() == 3 ? Vector4f(*v->Get3f(i), 1.0f) : *v->Get4f(i);
					m_tempVs.push_back(pos.xyz() * mat);
					m_tempV4.push_back(mat * pos);
				}
			}
		}
	}
}