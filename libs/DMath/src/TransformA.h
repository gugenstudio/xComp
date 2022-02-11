//==================================================================
/// TransformA.h
///
/// Created by Davide Pasca - 2016/5/26
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef TRANSFORMA_H
#define TRANSFORMA_H

#include <array>
#include "DMatrix33.h"
#include "DMatrix44.h"
#include "DQuat.h"

//#define TRANSFORMA_USE_MTX44

//==================================================================
using TransformAPacked = std::array<double,9+3>;

using TransformP = Matrix44T<float>;

#ifndef TRANSFORMA_USE_MTX44

//==================================================================
class TransformA
{
    using RealS = float;
    using RealL = double;

    Matrix33T<RealS>   mRotSca;
    Vec3<RealL>        mPos;

public:
    TransformA( bool doInitalize=false )
    {
        if ( doInitalize )
        {
            mRotSca = {true};
            mPos    = {0,0,0};
        }
    }

    TransformA( const Matrix33T<RealS> &rotSca, const Vec3<RealL> &pos )
        : mRotSca(rotSca)
        , mPos(pos)
    {
    }

    TransformA(
            const Quat &rotQua,
            const Vec3<RealS> &sca,
            const Vec3<RealL> &tra )
        : mRotSca( rotQua.ToMatrix33() * Matrix33T<RealS>::Scale( sca ) )
        , mPos(tra)
    {
    }

    TransformA( const Quat &rotQua )
        : mRotSca( rotQua.ToMatrix33() )
        , mPos(0,0,0)
    {}

    template <typename T>
    TransformA( const Matrix44T<T> &mtxAffine )
    {
        *this = mtxAffine;
    }

    TransformA( const TransformAPacked &packed )
    {
        for (size_t i=0; i != 3; ++i)
            for (size_t j=0; j != 3; ++j)
                mRotSca.mij(i,j) = (RealS)packed[i*3+j];

        for (size_t i=0; i != 3; ++i)
            mPos[i] = (RealL)packed[9 + i];
    }

    TransformAPacked MakePacked() const
    {
        TransformAPacked packed;
        for (size_t i=0; i != 3; ++i)
            for (size_t j=0; j != 3; ++j)
                packed[i*3+j] = mRotSca.mij(i,j);

        for (size_t i=0; i != 3; ++i)
            packed[9 + i] = mPos[i];

        return packed;
    }

    template <typename T>
    TransformA &operator=( const Matrix44T<T> &mtx44 );

    const Matrix33T<RealS> &Make33() const { return mRotSca; }

    Matrix44 Make44() const
    {
        auto out = Matrix44( mRotSca );
        out.mij(3,0) = (float)mPos[0];
        out.mij(3,1) = (float)mPos[1];
        out.mij(3,2) = (float)mPos[2];
        return out;
    }

    TransformP MakeTransformP() const
    {
        auto out = TransformP( mRotSca );
        out.mij(3,0) = (float)mPos[0];
        out.mij(3,1) = (float)mPos[1];
        out.mij(3,2) = (float)mPos[2];
        return out;
    }

    Vec3<RealS> CalcScale() const
    {
        return {
            DLength( mRotSca.GetV3( 0 ) ),
            DLength( mRotSca.GetV3( 1 ) ),
            DLength( mRotSca.GetV3( 2 ) )
        };
    }

    Quat CalcRotQua() const
    {
        return Quat( mRotSca.GetOrthonormal() );
    }

    //
    template <typename T>
    Vec3<T> GetTranslation() const {
        return { (T)mPos[0], (T)mPos[1], (T)mPos[2] };
    }

    const Vec3<RealL> &GetTranslation() const { return mPos; }

    //
    void SetTranslation( const Vec3<float> &pos )
    {
        mPos[0] = (RealL)pos[0];
        mPos[1] = (RealL)pos[1];
        mPos[2] = (RealL)pos[2];
    }
    void SetTranslation( const Vec3<double> &pos )
    {
        mPos[0] = (RealL)pos[0];
        mPos[1] = (RealL)pos[1];
        mPos[2] = (RealL)pos[2];
    }

    //
    void SetRotSca( const Matrix33 &rotSca ) { mRotSca = rotSca; }

    //
    TransformA GetAffineInverse() const
    {
        auto invRot = mRotSca.GetInverse();
        auto invTra = V3__V3_Mul_M33( -GetTranslation(), invRot );

        return { invRot, invTra };
    }

    TransformA GetOrthonormal() const
    {
        auto orthoRot = mRotSca.GetOrthonormal();
        auto tra = GetTranslation();

        return { orthoRot, tra };
    }

    // backward compatibility with Matrix44
    const Vec3<RealS> &GetV3( size_t rowIdx ) const
    {
        DASSERT( rowIdx <= 2 );
        return mRotSca.GetV3( rowIdx );
    }
    // backward compatibility with Matrix44
    void SetV3( size_t rowIdx, const Vec3<RealS> &rowVal )
    {
        DASSERT( rowIdx <= 2 );
        mRotSca.SetV3( rowIdx, rowVal );
    }

    TransformA GetAsRotSca() const { return TransformA( mRotSca, {0,0,0} ); }

    //
    static DMT_FINLINE TransformA Translate( const Vec3<double>& tra )
    {
        auto ret = TransformA(true);
        ret.SetTranslation( tra );
        return ret;
    }

    template <typename TANG>
    static TransformA Rot( const TANG& ang, const Vec3<RealS>& axis );

    static DMT_FINLINE TransformA Scale( const Vec3<RealS>& sca )
    {
        return TransformA( Matrix33T<RealS>::Scale( sca ), {0,0,0} );
    }

    //
    bool CheckValidity() const
    {
        return mRotSca == mRotSca && mPos == mPos;
    }

    bool IsSimilar( const TransformA &other, float eps ) const
    {
        return
            mRotSca.IsSimilar( other.mRotSca, eps ) &&
            mPos.IsSimilar( other.mPos, eps );
    }

    bool IsIdentity() const
    {
        return
            mRotSca.IsIdentity() &&
            mPos.IsSimilar( {0,0,0}, 0 );
    }

    //
    template <typename T>
    static TransformA LookAt(
                const Vec3<T> &eye,
                const Vec3<T> &at,
                const Vec3<T> &up,
                bool safeUpVec=true )
    {
        // build on the LookAt() from Matrix33
        auto mtx33 = Matrix33T<T>::LookAt( eye, at, up, safeUpVec );

        return { mtx33, eye };
    }

    friend TransformA operator * ( const TransformA &t1, const TransformA &t2 );

    template <typename T>
    friend Matrix44T<T> operator * ( const Matrix44T<T> &mtx1, const TransformA &t2 );

    template <typename T>
    friend TransformA operator * ( const TransformA &t1, const Matrix44T<T> &mtx2 );

    template <typename TV>
    friend Vec3<TV> DXFormPos( const Vec3<TV> &v, const TransformA &xform );
    template <typename TV>
    friend Vec3<TV> DXFormNor( const Vec3<TV> &v, const TransformA &xform );
    template <typename TV>
    friend Vec3<TV> DXFormNor( const TransformA &xform, const Vec3<TV> &v );
    template <typename TV>
    friend Vec3<TV> DXFormNorN( const Vec3<TV> &v, const TransformA &xform );
    template <typename TV>
    friend Vec3<TV> DXFormNorN( const TransformA &xform, const Vec3<TV> &v );
};

//==================================================================
template <typename T>
DMT_FINLINE TransformA &TransformA::operator=( const Matrix44T<T> &mtx44 )
{
    DMT_ASSERT(
        mtx44.mij(0,3) == 0 &&
        mtx44.mij(1,3) == 0 &&
        mtx44.mij(2,3) == 0 &&
        mtx44.mij(3,3) == 1 );

    mRotSca = Matrix33T<RealS>(
        (RealS)mtx44.mij(0,0), (RealS)mtx44.mij(0,1), (RealS)mtx44.mij(0,2),
        (RealS)mtx44.mij(1,0), (RealS)mtx44.mij(1,1), (RealS)mtx44.mij(1,2),
        (RealS)mtx44.mij(2,0), (RealS)mtx44.mij(2,1), (RealS)mtx44.mij(2,2) );

    mPos[0] = (RealL)mtx44.mij(3,0);
    mPos[1] = (RealL)mtx44.mij(3,1);
    mPos[2] = (RealL)mtx44.mij(3,2);

    return *this;
}

//==================================================================
DMT_FINLINE TransformA operator * ( const TransformA &t1, const TransformA &t2 )
{
    auto mtxT1_T2 = t1.mRotSca * t2.mRotSca;

    return {
        mtxT1_T2,
        V3__V3_Mul_M33( t1.mPos, t2.mRotSca ) + t2.mPos
    };
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> operator * ( const Matrix44T<T> &mtx1, const TransformA &t2 )
{
    return mtx1 * t2.Make44();
}

//==================================================================
template <typename T>
DMT_FINLINE TransformA operator * ( const TransformA &t1, const Matrix44T<T> &mtx2 )
{
    auto t2 = TransformA( mtx2 );
    return t1 * t2;
}

//==================================================================
template <typename TV>
DMT_FINLINE Vec3<TV> DXFormPos( const Vec3<TV> &v, const TransformA &xform )
{
    return
        V3__V3_Mul_M33<TV>( v, xform.Make33() ) + xform.mPos;
}
template <typename TV>
DMT_FINLINE Vec3<TV> DXFormPos( const TransformA &xform, const Vec3<TV> &v )
{
    return V3__M33_Mul_V3<TV>( xform.Make33(), v );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNor( const Vec3<TV> &v, const TransformA &xform )
{
    return V3__V3_Mul_M33<TV>( v, xform.mRotSca );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNor( const TransformA &xform, const Vec3<TV> &v )
{
    return V3__M33_Mul_V3<TV>( xform.mRotSca, v );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNorN( const Vec3<TV> &v, const TransformA &xform )
{
    return DNormalize( DXFormNor<TV>( v, xform ) );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNorN( const TransformA &xform, const Vec3<TV> &v )
{
    return DNormalize( DXFormNor<TV>( xform, v ) );
}

//==================================================================
template <typename TANG>
DMT_FINLINE TransformA TransformA::Rot( const TANG& ang, const Vec3<RealS>& axis )
{
    auto ret = TransformA(true);
    ret.mRotSca = Matrix33T<RealS>::Rot( (RealS)ang, axis );
    return ret;
}

//==================================================================
template <typename T>
DFORCEINLINE Matrix44T<T> Mul_TransformA_TransformP(
                                    const TransformA &a,
                                    const Matrix44T<T> &p )
{
    Matrix44T<T> out;

    const auto &a_rs = a.Make33();

    for (size_t r=0; r < 3; ++r)
    {
        for (size_t c=0; c < 4; ++c)
        {
            T sum;
            sum  = (T)a_rs.mij(r,0) * p.mij(0,c);
            sum += (T)a_rs.mij(r,1) * p.mij(1,c);
            sum += (T)a_rs.mij(r,2) * p.mij(2,c);

            out.mij(r,c) = sum;
        }
    }

    const auto &a_tra = a.GetTranslation();

    for (size_t c=0; c < 4; ++c)
    {
        double sum;
        sum  = a_tra[0] * (double)p.mij(0,c);
        sum += a_tra[1] * (double)p.mij(1,c);
        sum += a_tra[2] * (double)p.mij(2,c);
        sum +=            (double)p.mij(3,c);

        out.mij(3,c) = (T)sum;
    }

    return out;
}

#else // TRANSFORMA_USE_MTX44

//==================================================================
//==================================================================
//==================================================================
//==================================================================
class TransformA
{
    using RealS = float;
    using RealL = double;

    Matrix44           mMtx44;

public:
    TransformA( bool doInitalize=false )
    {
        if ( doInitalize )
            mMtx44 = {true};
    }

    TransformA( const Matrix33T<RealS> &rotSca, const Vec3<RealL> &pos )
        : mMtx44(rotSca)
    {
        mMtx44.SetTranslation( pos );
    }

    TransformA(
            const Quat &rotQua,
            const Vec3<RealS> &sca,
            const Vec3<RealL> &tra )
        : mMtx44( rotQua.ToMatrix33() * Matrix33T<RealS>::Scale( sca ) )
    {
        mMtx44.SetTranslation( tra );
    }

    TransformA( const Quat &rotQua )
        : mMtx44( rotQua.ToMatrix44() )
    {}

    template <typename T>
    TransformA( const Matrix44T<T> &mtxAffine )
    {
        *this = mtxAffine;
    }

    TransformA( const TransformAPacked &packed )
    {
        for (size_t i=0; i != 3; ++i)
            for (size_t j=0; j != 3; ++j)
                mMtx44.mij(i,j) = (RealS)packed[i*3+j];

        for (size_t i=0; i != 3; ++i)
            mPos[i] = (RealL)packed[9 + i];
    }

    TransformAPacked MakePacked() const
    {
        TransformAPacked packed;
        for (size_t i=0; i != 3; ++i)
            for (size_t j=0; j != 3; ++j)
                packed[i*3+j] = mMtx44.mij(i,j);

        for (size_t i=0; i != 3; ++i)
            packed[9 + i] = mMtx44.mij(3,i);

        return packed;
    }
    template <typename T> TransformA &operator=( const Matrix44T<T> &mtx44 );

    Matrix33T<RealS> Make33() const { return mMtx44.Make33(); }
    Matrix44         Make44() const { return mMtx44; }

    Vec3<RealS> CalcScale() const
    {
        return {
            DLength( mMtx44.GetV3( 0 ) ),
            DLength( mMtx44.GetV3( 1 ) ),
            DLength( mMtx44.GetV3( 2 ) )
        };
    }

    Quat CalcRotQua() const
    {
        return Quat( mMtx44.Make33().GetOrthonormal() );
    }

    //
    template <typename T>
    Vec3<T> GetTranslation() const {
        return { (T)mMtx44.mij(3,0), (T)mMtx44.mij(3,1), (T)mMtx44.mij(3,2) };
    }

    Vec3<RealL> GetTranslation() const { return mMtx44.GetTranslation(); }

    //
    void SetTranslation( const Vec3<float> &pos )
    {
        mMtx44.mij(3,0) = (RealS)pos[0];
        mMtx44.mij(3,1) = (RealS)pos[1];
        mMtx44.mij(3,2) = (RealS)pos[2];
    }
    void SetTranslation( const Vec3<double> &pos )
    {
        mMtx44.mij(3,0) = (RealS)pos[0];
        mMtx44.mij(3,1) = (RealS)pos[1];
        mMtx44.mij(3,2) = (RealS)pos[2];
    }

    //
    void SetRotSca( const Matrix33 &rotSca )
    {
        for (int i=0; i < 3; ++i)
            for (int j=0; j < 3; ++j)
                mMtx44.mij(i,j) = rotSca.mij(i,j);
    }

    //
    TransformA GetAffineInverse() const
    {
        auto invRot = mMtx44.Make33().GetInverse();
        auto invTra = V3__V3_Mul_M33( -GetTranslation(), invRot );

        return { invRot, invTra };
    }

    TransformA GetOrthonormal() const
    {
        auto orthoRot = mMtx44.Make33().GetOrthonormal();
        auto tra = GetTranslation();

        return { orthoRot, tra };
    }

    // backward compatibility with Matrix44
    const Vec3<RealS> &GetV3( size_t rowIdx ) const
    {
        DASSERT( rowIdx <= 2 );
        return mMtx44.GetV3( rowIdx );
    }
    // backward compatibility with Matrix44
    void SetV3( size_t rowIdx, const Vec3<RealS> &rowVal )
    {
        DASSERT( rowIdx <= 2 );
        mMtx44.SetV3( rowIdx, rowVal );
    }

    TransformA GetAsRotSca() const { return TransformA( mMtx44.GetAs33() ); }

    //
    static DMT_FINLINE TransformA Translate( const Vec3<double>& tra )
    {
        auto ret = TransformA(true);
        ret.SetTranslation( tra );
        return ret;
    }

    template <typename TANG>
    static TransformA Rot( const TANG& ang, const Vec3<RealS>& axis );

    static DMT_FINLINE TransformA Scale( const Vec3<RealS>& sca )
    {
        return Matrix44::Scale( {(RealS)sca[0], (RealS)sca[1], (RealS)sca[2]} );
    }

    //
    bool CheckValidity() const
    {
        return mMtx44 == mMtx44;
    }

    bool IsSimilar( const TransformA &other, float eps ) const
    {
        return mMtx44.IsSimilar( other.mMtx44, eps );
    }

    bool IsIdentity() const
    {
        return mMtx44.IsIdentity();
    }

    //
    template <typename T>
    static TransformA LookAt(
                const Vec3<T> &eye,
                const Vec3<T> &at,
                const Vec3<T> &up,
                bool safeUpVec=true )
    {
        // build on the LookAt() from Matrix33
        auto mtx33 = Matrix33T<T>::LookAt( eye, at, up, safeUpVec );

        auto mtx44 = Matrix44T<T>( mtx33 );
        mtx44.SetV3( 3, eye );

        return mtx44;
    }

    //
    friend TransformA operator * ( const TransformA &t1, const TransformA &t2 );

    template <typename T>
    friend Matrix44T<T> operator * ( const Matrix44T<T> &mtx1, const TransformA &t2 );

    template <typename T>
    friend TransformA operator * ( const TransformA &t1, const Matrix44T<T> &mtx2 );

    template <typename TV>
    friend Vec3<TV> DXFormPos( const Vec3<TV> &v, const TransformA &xform );
    template <typename TV>
    friend Vec3<TV> DXFormNor( const Vec3<TV> &v, const TransformA &xform );
    template <typename TV>
    friend Vec3<TV> DXFormNorN( const Vec3<TV> &v, const TransformA &xform );
};

//==================================================================
template <typename T>
DMT_FINLINE TransformA &TransformA::operator=( const Matrix44T<T> &mtx44 )
{
    DMT_ASSERT(
        mtx44.mij(0,3) == 0 &&
        mtx44.mij(1,3) == 0 &&
        mtx44.mij(2,3) == 0 &&
        mtx44.mij(3,3) == 1 );

    mMtx44 = mtx44;

    return *this;
}

//==================================================================
DMT_FINLINE TransformA operator * ( const TransformA &t1, const TransformA &t2 )
{
    return t1.mMtx44 * t2.mMtx44;
}

//==================================================================
template <typename T>
DMT_FINLINE Matrix44T<T> operator * ( const Matrix44T<T> &mtx1, const TransformA &t2 )
{
    return mtx1 * t2.mMtx44;
}

//==================================================================
template <typename T>
DMT_FINLINE TransformA operator * ( const TransformA &t1, const Matrix44T<T> &mtx2 )
{
    return t1.mMtx44 * mtx2;
}

//==================================================================
template <typename TV>
DMT_FINLINE Vec3<TV> DXFormPos( const Vec3<TV> &v, const TransformA &xform )
{
    //return
    //    V3__V3_Mul_M33<TV>( v, xform.Make33() ) + xform.GetTranslation<TV>();

    return V3__V3W1_Mul_M44<TV,float>( v, xform.mMtx44 );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNor( const Vec3<TV> &v, const TransformA &xform )
{
    return V3__V3W0_Mul_M44<TV>( v, xform.mMtx44 );
}

template <typename TV>
DMT_FINLINE Vec3<TV> DXFormNorN( const Vec3<TV> &v, const TransformA &xform )
{
    return DNormalize( DXFormNor<TV>( v, xform ) );
}

//==================================================================
template <typename TANG>
DMT_FINLINE TransformA TransformA::Rot( const TANG& ang, const Vec3<RealS>& axis )
{
    return Matrix44::Rot( (RealS)ang, axis );
}

#endif // TRANSFORMA_USE_MTX44

#endif

