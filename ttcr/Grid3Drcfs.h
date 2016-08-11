//
//  Grid3Drcfs.h
//  ttcr
//
//  Created by Bernard Giroux on 16-01-07.
//  Copyright © 2016 Bernard Giroux. All rights reserved.
//

#ifndef Grid3Drcfs_h
#define Grid3Drcfs_h

#include "Grid3Dri.h"
#include "Node3Di.h"

namespace ttcr {
    
    template<typename T1, typename T2>
    class Grid3Drcfs : public Grid3Dri<T1,T2,Node3Di<T1,T2>> {
    public:
        Grid3Drcfs(const T2 nx, const T2 ny, const T2 nz, const T1 ddx,
                   const T1 minx, const T1 miny, const T1 minz,
                   const T1 eps, const int maxit, const bool w,
                   const size_t nt=1) :
        Grid3Dri<T1,T2,Node3Di<T1,T2>>(nx, ny, nz, ddx, ddx, ddx, minx, miny, minz, nt),
        epsilon(eps), nitermax(maxit), niter(0), niterw(0), weno3(w)
        {
            buildGridNodes();
            this->buildGridNeighbors();
        }
        
        virtual ~Grid3Drcfs() {
        }
        
        int setSlowness(const std::vector<T1>& s);
        
        const int get_niter() const { return niter; }
        const int get_niterw() const { return niterw; }
        
        int raytrace(const std::vector<sxyz<T1>>& Tx,
                     const std::vector<T1>& t0,
                     const std::vector<sxyz<T1>>& Rx,
                     std::vector<T1>& traveltimes,
                     const size_t threadNo=0) const;
        
        int raytrace(const std::vector<sxyz<T1>>& Tx,
                     const std::vector<T1>& t0,
                     const std::vector<const std::vector<sxyz<T1>>*>& Rx,
                     std::vector<std::vector<T1>*>& traveltimes,
                     const size_t threadNo=0) const;
        
        int raytrace(const std::vector<sxyz<T1>>& Tx,
                     const std::vector<T1>& t0,
                     const std::vector<sxyz<T1>>& Rx,
                     std::vector<T1>& traveltimes,
                     std::vector<std::vector<sxyz<T1>>>& r_data,
                     const size_t threadNo=0) const;
        
        int raytrace(const std::vector<sxyz<T1>>& Tx,
                     const std::vector<T1>& t0,
                     const std::vector<const std::vector<sxyz<T1>>*>& Rx,
                     std::vector<std::vector<T1>*>& traveltimes,
                     std::vector<std::vector<std::vector<sxyz<T1>>>*>& r_data,
                     const size_t threadNo=0) const;
        
        int raytrace(const std::vector<sxyz<T1>>& Tx,
                     const std::vector<T1>& t0,
                     const std::vector<sxyz<T1>>& Rx,
                     std::vector<T1>& traveltimes,
                     std::vector<std::vector<sxyz<T1>>>& r_data,
                     std::vector<std::vector<siv<T1>>>& l_data,
                     const size_t threadNo=0) const;
        
    protected:
        T1 epsilon;
        int nitermax;
        mutable int niter;
        mutable int niterw;
        bool weno3;
        
        void buildGridNodes();
        
    private:
        Grid3Drcfs() {}
        Grid3Drcfs(const Grid3Drcfs<T1,T2>& g) {}
        Grid3Drcfs<T1,T2>& operator=(const Grid3Drcfs<T1,T2>& g) {}
        
    };
    
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::setSlowness(const std::vector<T1>& s) {
        
        if ( this->ncx*this->ncy*this->ncz != s.size() ) {
            std::cerr << "Error: slowness vectors of incompatible size.";
            return 1;
        }
        
        // interpolate slowness at grid nodes
        
        const size_t nx = this->ncx;
        const size_t ny = this->ncy;
        const size_t nz = this->ncz;
        
        // corners
        this->nodes[                       0].setNodeSlowness( s[                       0] );
        this->nodes[                      nx].setNodeSlowness( s[                    nx-1] );
        this->nodes[           ny *(nx+1)   ].setNodeSlowness( s[          (ny-1)*nx     ] );
        this->nodes[           ny *(nx+1)+nx].setNodeSlowness( s[          (ny-1)*nx+nx-1] );
        this->nodes[(nz*(ny+1)   )*(nx+1)   ].setNodeSlowness( s[((nz-1)*ny     )*nx     ] );
        this->nodes[(nz*(ny+1)   )*(nx+1)+nx].setNodeSlowness( s[((nz-1)*ny     )*nx+nx-1] );
        this->nodes[(nz*(ny+1)+ny)*(nx+1)   ].setNodeSlowness( s[((nz-1)*ny+ny-1)*nx     ] );
        this->nodes[(nz*(ny+1)+ny)*(nx+1)+nx].setNodeSlowness( s[((nz-1)*ny+ny-1)*nx+nx-1] );
        
        // edges
        for ( size_t i=1; i<nx; ++i ) {
            this->nodes[                      i].setNodeSlowness( 0.5*(s[                    i]+s[                    i-1]) );
            this->nodes[           ny *(nx+1)+i].setNodeSlowness( 0.5*(s[          (ny-1)*nx+i]+s[          (ny-1)*nx+i-1]) );
            this->nodes[(nz*(ny+1)   )*(nx+1)+i].setNodeSlowness( 0.5*(s[((nz-1)*ny     )*nx+i]+s[((nz-1)*ny     )*nx+i-1]) );
            this->nodes[(nz*(ny+1)+ny)*(nx+1)+i].setNodeSlowness( 0.5*(s[((nz-1)*ny+ny-1)*nx+i]+s[((nz-1)*ny+ny-1)*nx+i-1]) );
        }
        for ( size_t j=1; j<ny; ++j ) {
            this->nodes[           j *(nx+1)   ].setNodeSlowness( 0.5*(s[            j*nx     ]+s[          (j-1)*nx     ]) );
            this->nodes[           j *(nx+1)+nx].setNodeSlowness( 0.5*(s[            j*nx+nx-1]+s[          (j-1)*nx+nx-1]) );
            this->nodes[(nz*(ny+1)+j)*(nx+1)   ].setNodeSlowness( 0.5*(s[((nz-1)*ny+j)*nx     ]+s[((nz-1)*ny+j-1)*nx     ]) );
            this->nodes[(nz*(ny+1)+j)*(nx+1)+nx].setNodeSlowness( 0.5*(s[((nz-1)*ny+j)*nx+nx-1]+s[((nz-1)*ny+j-1)*nx+nx-1]) );
        }
        for ( size_t k=1; k<nz; ++k ) {
            this->nodes[(k*(ny+1)   )*(nx+1)   ].setNodeSlowness( 0.5*(s[(k*ny     )*nx     ]+s[((k-1)*ny     )*nx     ]) );
            this->nodes[(k*(ny+1)   )*(nx+1)+nx].setNodeSlowness( 0.5*(s[(k*ny     )*nx+nx-1]+s[((k-1)*ny     )*nx+nx-1]) );
            this->nodes[(k*(ny+1)+ny)*(nx+1)   ].setNodeSlowness( 0.5*(s[(k*ny+ny-1)*nx     ]+s[((k-1)*ny+ny-1)*nx     ]) );
            this->nodes[(k*(ny+1)+ny)*(nx+1)+nx].setNodeSlowness( 0.5*(s[(k*ny+ny-1)*nx+nx-1]+s[((k-1)*ny+ny-1)*nx+nx-1]) );
        }
        
        // faces
        for ( size_t i=1; i<nx; ++i ) {
            for ( size_t j=1; j<ny; ++j ) {
                this->nodes[           j *(nx+1)+i].setNodeSlowness( 0.25*(s[             j *nx+i]+s[             j *nx+i-1]+
                                                                           s[          (j-1)*nx+i]+s[          (j-1)*nx+i-1]) );
                this->nodes[(nz*(ny+1)+j)*(nx+1)+i].setNodeSlowness( 0.25*(s[((nz-1)*ny+  j)*nx+i]+s[((nz-1)*ny+  j)*nx+i-1]+
                                                                           s[((nz-1)*ny+j-1)*nx+i]+s[((nz-1)*ny+j-1)*nx+i-1]) );
            }
        }
        for ( size_t i=1; i<nx; ++i ) {
            for ( size_t k=1; k<nz; ++k ) {
                this->nodes[(k*(ny+1)   )*(nx+1)+i].setNodeSlowness( 0.25*(s[(   k *ny     )*nx+i]+s[(   k *ny     )*nx+i-1]+
                                                                           s[((k-1)*ny     )*nx+i]+s[((k-1)*ny     )*nx+i-1]) );
                this->nodes[(k*(ny+1)+ny)*(nx+1)+i].setNodeSlowness( 0.25*(s[(   k *ny+ny-1)*nx+i]+s[(   k *ny+ny-1)*nx+i-1]+
                                                                           s[((k-1)*ny+ny-1)*nx+i]+s[((k-1)*ny+ny-1)*nx+i-1]) );
            }
        }
        for ( size_t j=1; j<ny; ++j ) {
            for ( size_t k=1; k<nz; ++k ) {
                this->nodes[(k*(ny+1)+j)*(nx+1)   ].setNodeSlowness( 0.25*(s[(k*ny+  j)*nx     ]+s[((k-1)*ny+  j)*nx     ]+
                                                                           s[(k*ny+j-1)*nx     ]+s[((k-1)*ny+j-1)*nx     ]) );
                this->nodes[(k*(ny+1)+j)*(nx+1)+nx].setNodeSlowness( 0.25*(s[(k*ny+  j)*nx+nx-1]+s[((k-1)*ny+  j)*nx+nx-1]+
                                                                           s[(k*ny+j-1)*nx+nx-1]+s[((k-1)*ny+j-1)*nx+nx-1]) );
            }
        }
        
        // interior
        for ( size_t i=1; i<nx; ++i ) {
            for ( size_t j=1; j<ny; ++j ) {
                for ( size_t k=1; k<nz; ++k ) {
                    this->nodes[(k*(ny+1)+j)*(nx+1)+i].setNodeSlowness( 0.125*(s[(    k*ny+j  )*nx+i  ]+
                                                                               s[(    k*ny+j  )*nx+i-1]+
                                                                               s[(    k*ny+j-1)*nx+i  ]+
                                                                               s[(    k*ny+j-1)*nx+i-1]+
                                                                               s[((k-1)*ny+j  )*nx+i  ]+
                                                                               s[((k-1)*ny+j  )*nx+i-1]+
                                                                               s[((k-1)*ny+j-1)*nx+i  ]+
                                                                               s[((k-1)*ny+j-1)*nx+i-1]) );
                }
            }
        }
        return 0;
    }
    
    
    template<typename T1, typename T2>
    void Grid3Drcfs<T1,T2>::buildGridNodes() {
        
        T2 cell_XmYmZm; 	// cell in the (x-,y-,z-) direction from the node
        T2 cell_XpYmZm; 	// cell in the (x+,y-,z-) direction from the node
        T2 cell_XmYpZm;
        T2 cell_XpYpZm;
        T2 cell_XmYmZp;
        T2 cell_XpYmZp;
        T2 cell_XmYpZp;
        T2 cell_XpYpZp;
        
        T2 n=0;
        for ( T2 nk=0; nk<=this->ncz; ++nk ) {
            
            T1 z = this->zmin + nk*this->dz;
            
            for ( T2 nj=0; nj<=this->ncy; ++nj ) {
                
                T1 y = this->ymin + nj*this->dy;
                
                for (T2 ni=0; ni<=this->ncx; ++ni){
                    
                    T1 x = this->xmin + ni*this->dx;
                    
                    // Find the adjacent cells for each primary node
                    
                    if (ni < this->ncx && nj < this->ncy && nk < this->ncz){
                        cell_XpYpZp = nj*this->ncx + nk*(this->ncx*this->ncy) + ni;
                    }
                    else {
                        cell_XpYpZp = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni > 0 && nj < this->ncy && nk < this->ncz){
                        cell_XmYpZp = nj*this->ncx + nk*(this->ncx*this->ncy) + ni - 1;
                    }
                    else {
                        cell_XmYpZp = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni < this->ncx && nj > 0 && nk < this->ncz){
                        cell_XpYmZp = (nj-1)*this->ncx + nk*(this->ncx*this->ncy) + ni;
                    }
                    else {
                        cell_XpYmZp = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni > 0 && nj > 0 && nk < this->ncz){
                        cell_XmYmZp = (nj-1)*this->ncx + nk*(this->ncx * this->ncy) + ni - 1;
                    }
                    else {
                        cell_XmYmZp = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni < this->ncx && nj < this->ncy && nk > 0){
                        cell_XpYpZm = nj*this->ncx + (nk-1)*(this->ncx*this->ncy) + ni;
                    }
                    else {
                        cell_XpYpZm = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni > 0 && nj < this->ncy && nk > 0){
                        cell_XmYpZm = nj*this->ncx + (nk-1)*(this->ncx*this->ncy) + ni - 1;
                    }
                    else {
                        cell_XmYpZm = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni < this->ncx && nj > 0 && nk > 0){
                        cell_XpYmZm = (nj-1)*this->ncx + (nk-1)*(this->ncx*this->ncy) + ni;
                    }
                    else {
                        cell_XpYmZm = std::numeric_limits<T2>::max();
                    }
                    
                    if (ni > 0 && nj > 0 && nk > 0){
                        cell_XmYmZm = (nj-1)*this->ncx + (nk-1)*(this->ncx*this->ncy) + ni - 1;
                    }
                    else {
                        cell_XmYmZm = std::numeric_limits<T2>::max();
                    }
                    
                    
                    // Index the primary nodes owners
                    
                    if (cell_XmYmZm != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XmYmZm );
                    }
                    if (cell_XpYmZm != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XpYmZm );
                    }
                    if (cell_XmYpZm != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XmYpZm );
                    }
                    if (cell_XpYpZm != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XpYpZm );
                    }
                    if (cell_XmYmZp != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XmYmZp );
                    }
                    if (cell_XpYmZp != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XpYmZp );
                    }
                    if (cell_XmYpZp != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XmYpZp );
                    }
                    if (cell_XpYpZp != std::numeric_limits<T2>::max() ) {
                        this->nodes[n].pushOwner( cell_XpYpZp );
                    }
                    
                    this->nodes[n].setXYZindex( x, y, z, n );
                    
                    ++n;
                }
            }
        }
    }
    
    
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::raytrace(const std::vector<sxyz<T1>>& Tx,
                                    const std::vector<T1>& t0,
                                    const std::vector<sxyz<T1>>& Rx,
                                    std::vector<T1>& traveltimes,
                                    const size_t threadNo) const {
        
        if ( this->checkPts(Tx) == 1 ) return 1;
        if ( this->checkPts(Rx) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        // Set Tx pts
        std::vector<bool> frozen( this->nodes.size(), false );
        int npts = 1;
        if ( weno3 == true) npts = 2;
        this->initFSM(Tx, t0, frozen, npts, threadNo);
        
        std::vector<T1> times( this->nodes.size() );
        for ( size_t n=0; n<this->nodes.size(); ++n )
            times[n] = this->nodes[n].getTT( threadNo );
        
        T1 change = std::numeric_limits<T1>::max();
        if ( weno3 == true ) {
            niter=0;
            niterw=0;
            if ( this->dx != this->dz || this->dx != this->dy ) {
                std::cerr << "Error: WENO stencil needs dx equal to dz" << std::endl;
                abort();
            }
            while ( change >= epsilon && niter<nitermax ) {
                this->sweep(frozen, threadNo);
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niter++;
            }
            change = std::numeric_limits<T1>::max();
            while ( change >= epsilon && niterw<nitermax ) {
                this->sweep_weno3(frozen, threadNo);
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niterw++;
            }
        } else {
            niter=0;
            while ( change >= epsilon && niter<nitermax ) {
                this->sweep(frozen, threadNo);
                
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niter++;
            }
        }
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        
        for (size_t n=0; n<Rx.size(); ++n) {
            traveltimes[n] = this->getTraveltime(Rx[n], threadNo);
        }
        return 0;
    }
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::raytrace(const std::vector<sxyz<T1>>& Tx,
                                    const std::vector<T1>& t0,
                                    const std::vector<const std::vector<sxyz<T1>>*>& Rx,
                                    std::vector<std::vector<T1>*>& traveltimes,
                                    const size_t threadNo) const {
        
        if ( this->checkPts(Tx) == 1 ) return 1;
        for ( size_t n=0; n<Rx.size(); ++n )
            if ( this->checkPts(*Rx[n]) == 1 ) return 1;
        
        for ( size_t n=0; n<this->nodes.size(); ++n ) {
            this->nodes[n].reinit( threadNo );
        }
        
        if ( traveltimes.size() != Rx.size() ) {
            traveltimes.resize( Rx.size() );
        }
        
        // Set Tx pts
        std::vector<bool> frozen( this->nodes.size(), false );
        int npts = 1;
        if ( weno3 == true) npts = 2;
        this->initFSM(Tx, t0, frozen, npts, threadNo);
        
        std::vector<T1> times( this->nodes.size() );
        for ( size_t n=0; n<this->nodes.size(); ++n )
            times[n] = this->nodes[n].getTT( threadNo );
        
        T1 change = std::numeric_limits<T1>::max();
        if ( weno3 == true ) {
            niter=0;
            niterw=0;
            if ( this->dx != this->dz || this->dx != this->dy ) {
                std::cerr << "Error: WENO stencil needs dx equal to dz" << std::endl;
                abort();
            }
            while ( change >= epsilon && niter<nitermax ) {
                this->sweep(frozen, threadNo);
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niter++;
            }
            change = std::numeric_limits<T1>::max();
            while ( change >= epsilon && niterw<nitermax ) {
                this->sweep_weno3(frozen, threadNo);
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niterw++;
            }
        } else {
            niter=0;
            while ( change >= epsilon && niter<nitermax ) {
                this->sweep(frozen, threadNo);
                
                change = 0.0;
                for ( size_t n=0; n<this->nodes.size(); ++n ) {
                    T1 dt = fabs( times[n] - this->nodes[n].getTT(threadNo) );
                    
                    change += dt;
                    times[n] = this->nodes[n].getTT(threadNo);
                }
                niter++;
            }
        }
        
        for (size_t nr=0; nr<Rx.size(); ++nr) {
            traveltimes[nr]->resize( Rx[nr]->size() );
            for (size_t n=0; n<Rx[nr]->size(); ++n)
                (*traveltimes[nr])[n] = this->getTraveltime((*Rx[nr])[n], threadNo);
        }
        return 0;
    }
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::raytrace(const std::vector<sxyz<T1>>& Tx,
                                    const std::vector<T1>& t0,
                                    const std::vector<sxyz<T1>>& Rx,
                                    std::vector<T1>& traveltimes,
                                    std::vector<std::vector<sxyz<T1>>>& r_data,
                                    const size_t threadNo) const {
        
        int check = raytrace(Tx, t0, Rx, traveltimes, threadNo);
        if ( check == 1 ) return 1;
        
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<r_data.size(); ++ni ) {
            r_data[ni].resize( 0 );
        }
        
        for (size_t n=0; n<Rx.size(); ++n) {
            this->getRaypath(Tx, Rx[n], r_data[n], threadNo);
        }
        return 0;
    }
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::raytrace(const std::vector<sxyz<T1>>& Tx,
                                    const std::vector<T1>& t0,
                                    const std::vector<const std::vector<sxyz<T1>>*>& Rx,
                                    std::vector<std::vector<T1>*>& traveltimes,
                                    std::vector<std::vector<std::vector<sxyz<T1>>>*>& r_data,
                                    const size_t threadNo) const {
        
        int check = raytrace(Tx, t0, Rx, traveltimes, threadNo);
        if ( check == 1 ) return 1;
        
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        
        for (size_t nr=0; nr<Rx.size(); ++nr) {
            r_data[nr]->resize( Rx[nr]->size() );
            for ( size_t ni=0; ni<r_data[nr]->size(); ++ni ) {
                (*r_data[nr])[ni].resize( 0 );
            }
            
            for (size_t n=0; n<Rx[nr]->size(); ++n) {
                this->getRaypath(Tx, (*Rx[nr])[n], (*r_data[nr])[n], threadNo);
            }
        }
        return 0;
    }
    
    template<typename T1, typename T2>
    int Grid3Drcfs<T1,T2>::raytrace(const std::vector<sxyz<T1>>& Tx,
                                    const std::vector<T1>& t0,
                                    const std::vector<sxyz<T1>>& Rx,
                                    std::vector<T1>& traveltimes,
                                    std::vector<std::vector<sxyz<T1>>>& r_data,
                                    std::vector<std::vector<siv<T1>>>& l_data,
                                    const size_t threadNo) const {
        
        int check = raytrace(Tx, t0, Rx, traveltimes, threadNo);
        if ( check == 1 ) return 1;
        
        if ( r_data.size() != Rx.size() ) {
            r_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<r_data.size(); ++ni ) {
            r_data[ni].resize( 0 );
        }
        if ( l_data.size() != Rx.size() ) {
            l_data.resize( Rx.size() );
        }
        for ( size_t ni=0; ni<l_data.size(); ++ni ) {
            l_data[ni].resize( 0 );
        }
        
        siv<T1> cell;
        for (size_t n=0; n<Rx.size(); ++n) {
            this->getRaypath(Tx, Rx[n], r_data[n], threadNo);
            
            for (size_t ns=0; ns<r_data[n].size()-1; ++ns) {
                sxyz<T1> m = 0.5*(r_data[n][ns]+r_data[n][ns+1]);  // ps @ middle of segment
                cell.i = this->getCellNo( m );
                cell.v = r_data[n][ns].getDistance( r_data[n][ns+1] );
                
                bool found=false;
                for (size_t nc=0; nc<l_data[n].size(); ++nc) {
                    if ( l_data[n][nc].i == cell.i ) {
                        l_data[n][nc].v += cell.v;  // must add in case we pass through secondary nodes along edge
                        found = true;
                        break;
                    }
                }
                if ( found == false ) {
                    l_data[n].push_back( cell );
                }
            }
            //  must be sorted to build matrix L
            sort(l_data[n].begin(), l_data[n].end(), CompareSiv_i<T1>());
            
        }
        return 0;
    }
    
}

#endif /* Grid3Drcfs_h */
