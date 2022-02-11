//==================================================================
/// DFunctionsList.h
///
/// Created by Davide Pasca - 2015/10/6
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef DFUNCTIONSLIST_H
#define DFUNCTIONSLIST_H

#include <algorithm>
#include "DContainers.h"

//==================================================================
using DFuncListHandType = int;
using DFuncListHandle = sptr<DFuncListHandType>;

//==================================================================
template <typename T>
class DFunctionsListBase
{
protected:
    static const size_t MAX_FUNCS = 64;

    struct Item
    {
        wptr<DFuncListHandType> hand;
        T                      func;

        Item() {}

        Item( sptr<DFuncListHandType> &h, const T &f )
            : hand(h)
            , func(f)
        {
        }
    };

    DVec<Item>              mItems;
    DFun<void (const T &)>  mOnAddCallFn;

public:
    void SetAddCallAuditor( DFun<void (T)> onAddCallFn )
    {
        mOnAddCallFn = onAddCallFn;
    }

    DFuncListHandle AddNewCall( const T &func )
    {
        auto hand = std::make_shared<DFuncListHandType>();
        AddCall( hand, func, false );
        return hand;
    }

    DFuncListHandle AddNewCallFront( const T &func )
    {
        auto hand = std::make_shared<DFuncListHandType>();
        AddCall( hand, func, true );
        return hand;
    }

    void AddCall( sptr<DFuncListHandType> &hand, const T &func, bool addToFront )
    {
        if ( mItems.size() >= MAX_FUNCS )
        {
            DASSERT( 0 );
            return;
        }

        if ( addToFront )
            mItems.insert( mItems.begin(), {hand, func} );
        else
            mItems.emplace_back( hand, func );

        if ( mOnAddCallFn )
            mOnAddCallFn( func );
    }

    void MoveCallFront( const DFuncListHandle &hand )
    {
        auto it = std::find_if(
                mItems.begin(),
                mItems.end(),
                [&](const auto &c) { return c.hand.lock() == hand; } );

        std::rotate( mItems.begin(), it, it+1 );
    }
};

//==================================================================
template <typename T>
class DFunctionsList : public DFunctionsListBase<T>
{
public:
    template <class ...Args>
    void CallFuncs( Args&&... args )
    {
        auto &items = this->mItems;
        items.erase(
            std::remove_if(
                items.begin(), items.end(),
                []( const auto &i ){ return i.hand.expired(); } ),
                items.end() );

        for (auto &item : items)
            if NOT( item.hand.expired() )
                item.func( args... );
    }

    template <class ...Args>
    void CallFunction( DFuncListHandle hand, Args&&... args )
    {
        auto &items = this->mItems;
        auto it = std::find_if(
                items.begin(),
                items.end(),
                [&](const auto &c) { return c.hand.lock() == hand; } );

        it->func( args... );
    }
};

//==================================================================
template <typename T>
class DFunListRetCheck : public DFunctionsListBase<T>
{
public:
    template <class ...Args>
    bool CallFuncs( Args&&... args )
    {
        auto &items = this->mItems;
        items.erase(
            std::remove_if(
                items.begin(), items.end(),
                []( const auto &i ){ return i.hand.expired(); } ),
                items.end() );

        for (auto &item : items)
            if NOT( item.hand.expired() )
                if ( item.func( args... ) )
                    return true;

        return false;
    }
};

#endif

