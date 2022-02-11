//==================================================================
/// BCUIUT_OrdersMap.h
///
/// Created by Davide Pasca - 2020/10/04
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef BCUIUT_ORDERSMAP_H
#define BCUIUT_ORDERSMAP_H

#include <unordered_map>

//==================================================================
template <typename T>
class BCUIUT_OrdersMap
{
public:
    std::unordered_map<int64_t, T> mMap;

    void UpdateOrderMap( const DVec<T> &srcOrds )
    {
        // update the map if necessary
        if ( mMap.size() != srcOrds.size() )
            for (c_auto &ord : srcOrds)
                mMap.insert({ ord.orderId, ord });
    }

    const T *FindOrderByTradeID( size_t tradeID ) const
    {
        if (c_auto it  = mMap.find( (int64_t)tradeID ); it != mMap.end())
            return &it->second;

        return nullptr;
    }

    void ClearOrderMap()
    {
        mMap = {};
    }
};


#endif

