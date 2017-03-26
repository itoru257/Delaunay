/*************************************************
 * Copyright (c) 2017 Toru Ito
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 *************************************************/

#ifndef Delaunay_hpp
#define Delaunay_hpp

#include <stdio.h>
#include <vector>
#include <set>
#include <Eigen/Core>
#include <Eigen/Geometry>

class Delaunay
{
public:
    struct TriangleData
    {
        unsigned long   index1;
        unsigned long   index2;
        unsigned long   index3;
        double          radius;
        double          bounds[4];
        Eigen::Vector2d center;
        
        TriangleData()
        {
            index1 = index2 = index3 = 0;
            bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
            radius = 0.0;
            center = Eigen::Vector2d::Zero();
        }

        bool IsInside( Eigen::Vector2d point )
        {
            if( point.x() < bounds[0] )
                return false;

            if( point.x() > bounds[1] )
                return false;

            if( point.y() < bounds[2] )
                return false;

            if( point.y() > bounds[3] )
                return false;

            if( ( point - center ).norm() > radius )
                return false;
        
            return true;
        }
    };
    
    struct EdgeData
    {
        unsigned long index1, index2;
        
        EdgeData()
        {
            index1 = 0;
            index2 = 0;
        };
        
        EdgeData( unsigned long index1, unsigned long index2 )
        {
            if( index1 < index2 ) {
                this->index1 = index1;
                this->index2 = index2;
            } else {
                this->index1 = index2;
                this->index2 = index1;
            }
        };
    };
    
public:
    Delaunay();
    ~Delaunay();

    void SetPoint( std::vector< Eigen::Vector2d > *pPointList );
    void GetResult( std::vector< Eigen::Vector2d > *pPointList, std::vector< std::vector< unsigned int > > *pIndexList );
    
    void Triangulation();
    
private:
    std::vector< TriangleData >    m_Triangles;
    std::vector< Eigen::Vector2d > m_Points;
    std::vector< unsigned long >   m_InitTrianglePointIndex;
    
    void CreateInitTriangle();
    void DeleteInitTriangle();
    
    void AddTriangle( unsigned long index1, unsigned long index2, unsigned long index3 );
};

#endif /* Delaunay_hpp */
