//
//  Node3Dcsp.h
//  ttcr
//
//  Created by Bernard Giroux on 2014-03-03.
//  Copyright (c) 2014 Bernard Giroux. All rights reserved.
//

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ttcr_Node3Dcsp_h
#define ttcr_Node3Dcsp_h

#include <cmath>
#include <limits>

#include "Node.h"

namespace ttcr {

    template<typename T1, typename T2>
    class Node3Dcsp : public Node<T1> {
    public:
        Node3Dcsp(const size_t nt=1) :
        nThreads(nt),
        x(0.0f), y(0.0f), z(0.0f),
        gridIndex(std::numeric_limits<T2>::max()),
        tt(nullptr),
        nodeParent(nullptr),
        cellParent(nullptr),
        owners(0),
        primary(false)
        {
            tt = new T1[nt];
            nodeParent = new T2[nt];
            cellParent = new T2[nt];

            for ( size_t n=0; n<nt; ++n ) {
                tt[n] = std::numeric_limits<T1>::max();
                nodeParent[n] = std::numeric_limits<T2>::max();
                cellParent[n] = std::numeric_limits<T2>::max();
            }
        }

        Node3Dcsp(const T1 xx, const T1 yy, const T1 zz, const T2 index,
                  const size_t nt) :
        nThreads(nt),
        x(xx), y(yy), z(zz),
        gridIndex(index),
        tt(nullptr),
        nodeParent(nullptr),
        cellParent(nullptr),
        owners(0),
        primary(false)
        {
            tt = new T1[nt];
            nodeParent = new T2[nt];
            cellParent = new T2[nt];

            for ( size_t n=0; n<nt; ++n ) {
                tt[n] = std::numeric_limits<T1>::max();
                nodeParent[n] = std::numeric_limits<T2>::max();
                cellParent[n] = std::numeric_limits<T2>::max();
            }
        }

        Node3Dcsp(const T1 t, const T1 xx, const T1 yy, const T1 zz, const size_t nt,
                  const size_t i) :
        nThreads(nt),
        x(xx), y(yy), z(zz),
        gridIndex(std::numeric_limits<T2>::max()),
        tt(nullptr),
        nodeParent(nullptr),
        cellParent(nullptr),
        owners(0),
        primary(false)
        {
            tt = new T1[nt];
            nodeParent = new T2[nt];
            cellParent = new T2[nt];

            for ( size_t n=0; n<nt; ++n ) {
                tt[n] = std::numeric_limits<T1>::max();
                nodeParent[n] = std::numeric_limits<T2>::max();
                cellParent[n] = std::numeric_limits<T2>::max();
            }
            tt[i]=t;
        }

        Node3Dcsp(const T1 t, const sxyz<T1> &s, const size_t nt, const size_t i) :
        nThreads(nt),
        x(s.x), y(s.y), z(s.z),
        gridIndex(std::numeric_limits<T2>::max()),
        tt(nullptr),
        nodeParent(nullptr),
        cellParent(nullptr),
        owners(0),
        primary(false)
        {
            tt = new T1[nt];
            nodeParent = new T2[nt];
            cellParent = new T2[nt];

            for ( size_t n=0; n<nt; ++n ) {
                tt[n] = std::numeric_limits<T1>::max();
                nodeParent[n] = std::numeric_limits<T2>::max();
                cellParent[n] = std::numeric_limits<T2>::max();
            }
            tt[i]=t;
        }

        Node3Dcsp(const Node3Dcsp<T1,T2>& node) :
        nThreads(node.nThreads),
        x(node.x), y(node.y), z(node.z),
        gridIndex(node.gridIndex),
        tt(nullptr),
        nodeParent(nullptr),
        cellParent(nullptr),
        owners(node.owners),
        primary(node.primary)
        {
            tt = new T1[nThreads];
            nodeParent = new T2[nThreads];
            cellParent = new T2[nThreads];

            for ( size_t n=0; n<nThreads; ++n ) {
                tt[n] = node.tt[n];
                nodeParent[n] = node.nodeParent[n];
                cellParent[n] = node.cellParent[n];
            }
        }

        virtual ~Node3Dcsp() {
            delete [] tt;
            delete [] nodeParent;
            delete [] cellParent;
        }

        // Sets the vectors to the right size of threads and initialize it
        void reinit(const size_t n) {
            tt[n] = std::numeric_limits<T1>::max();
            if ( nodeParent != nullptr )
                nodeParent[n] = std::numeric_limits<T2>::max();
            if ( cellParent != nullptr )
                cellParent[n] = std::numeric_limits<T2>::max();
        }

        T1 getTT(const size_t n) const { return tt[n]; }
        void setTT(const T1 t, const size_t n ) { tt[n] = t; }

        void setXYZindex(const T1 xx, const T1 yy, const T1 zz, const T2 index) {
            x=xx; y=yy; z=zz; gridIndex = index;  }

        void setXYZindex(const sxyz<T1> &s, const T2 index) {
            x=s.x; y=s.y; z=s.z; gridIndex = index;  }

        T1 getX() const { return x; }
        void setX(const T1 xx) { x = xx; }

        T1 getY() const { return y; }
        void setY(const T1 yy) { y = yy; }

        T1 getZ() const { return z; }
        void setZ(const T1 zz) { z = zz; }

        T2 getGridIndex() const { return gridIndex; }
        void setGridIndex(const T2 index) { gridIndex = index; }

        T2 getNodeParent(const size_t n) const { return nodeParent[n]; }
        void setnodeParent(const T2 index, const size_t n) { nodeParent[n] = index; }

        T2 getCellParent(const size_t n) const { return cellParent[n]; }
        void setCellParent(const T2 index, const size_t n) { cellParent[n] = index; }

        void pushOwner(const T2 o) { owners.push_back(o); }
        const std::vector<T2>& getOwners() const { return owners; }

        T1 getDistance( const Node3Dcsp<T1,T2>& node ) const {
            return sqrt( (x-node.x)*(x-node.x) + (y-node.y)*(y-node.y) + (z-node.z)*(z-node.z) );
        }

        T1 getDistance( const sxyz<T1>& node ) const {
            return sqrt( (x-node.x)*(x-node.x) + (y-node.y)*(y-node.y) + (z-node.z)*(z-node.z) );
        }

        // operator to test if same location
        bool operator==( const sxyz<T1>& node ) const {
            return std::abs(x-node.x)<small && std::abs(y-node.y)<small && std::abs(z-node.z)<small;
        }

        Node3Dcsp<T1, T2>& operator-=(const sxyz<T1>& node) {
            this->x -= node.x;
            this->y -= node.y;
            this->z -= node.z;
            return *this;
        }

        size_t getSize() const {
            return sizeof(size_t) + nThreads*sizeof(T1) + 3*sizeof(T1) +
            (1+2*nThreads)*sizeof(T2) + owners.size() * sizeof(T2);
        }

        int getDimension() const { return 3; }

        void setPrimary(const bool p) { primary = p; }
        const bool isPrimary() const { return primary; }

    private:
        size_t nThreads;
        T1 x;                       // x coordinate [km]
        T1 y;						// y coordinate [km]
        T1 z;                       // z coordinate [km]
        T2 gridIndex;               // index of this node in the list of the grid
        T1 *tt;                     // travel time for the multiple source points
        T2 *nodeParent;             // index of parent node of the ray for each thread
        T2 *cellParent;             // index of cell traversed by the ray for each thread
        std::vector<T2> owners;     // indices of cells touching the node
        bool primary;

    };

    template<typename T1, typename T2>
    std::ostream& operator<< (std::ostream& os, const Node3Dcsp<T1, T2> &n) {
        os << n.getX() << ' ' << n.getY() << ' ' << n.getZ();
        return os;
    }

    template<typename T1, typename T2>
    sxyz<T1> operator-(const Node3Dcsp<T1,T2>& lhs, const Node3Dcsp<T1,T2>& rhs) {
        return sxyz<T1>( lhs.getX()-rhs.getX(), lhs.getY()-rhs.getY(), lhs.getZ()-rhs.getZ() );
    }

    template<typename T1, typename T2>
    sxyz<T1> operator-(const sxyz<T1>& lhs, const Node3Dcsp<T1,T2>& rhs) {
        return sxyz<T1>( lhs.x-rhs.getX(), lhs.y-rhs.getY(), lhs.z-rhs.getZ() );
    }

    template<typename T1, typename T2>
    sxyz<T1> operator-(const Node3Dcsp<T1,T2>& lhs, const sxyz<T1>& rhs) {
        return sxyz<T1>( lhs.getX()-rhs.x, lhs.getY()-rhs.y, lhs.getZ()-rhs.z );
    }

}

#endif
