/*************************************************
 * Copyright (c) 2017 Toru Ito
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 *************************************************/

#include "Delaunay.hpp"
#include <cmath>

Delaunay::Delaunay()
{
}

Delaunay::~Delaunay()
{
}

void Delaunay::SetPoint( std::vector< Eigen::Vector2d > *pPointList )
{
    m_Points.clear();
    std::copy( pPointList->begin(), pPointList->end(), std::back_inserter( m_Points ) );
}

void Delaunay::GetResult( std::vector< Eigen::Vector2d > *pPointList, std::vector< std::vector< unsigned int > > *pIndexList )
{
    pPointList->clear();
    pIndexList->clear();

    std::vector< unsigned int > indexs;
    indexs.resize(3);

    for( auto tri : m_Triangles ) {
        indexs[0] = static_cast< unsigned int >( tri.index1 );
        indexs[1] = static_cast< unsigned int >( tri.index2 );
        indexs[2] = static_cast< unsigned int >( tri.index3 );
        pIndexList->push_back( indexs );
    }

    std::copy( m_Points.begin(), m_Points.end(), std::back_inserter( *pPointList ) );
}

void Delaunay::CreateInitTriangle()
{
    m_Triangles.clear();

    unsigned long PointSize = m_Points.size();
    double x_min, x_max, y_min, y_max, x_delta, y_delta;
    
    x_min = m_Points.front().x();
    x_max = m_Points.front().x();
    y_min = m_Points.front().y();
    y_max = m_Points.front().y();
    
    for( auto point : m_Points ) {
        x_min = ( x_min < point.x() ) ? x_min : point.x();
        x_max = ( x_max > point.x() ) ? x_max : point.x();
        y_min = ( y_min < point.y() ) ? y_min : point.y();
        y_max = ( y_max > point.y() ) ? y_max : point.y();
    }
    
    x_delta = x_max - x_min;
    y_delta = y_max - y_min;
    
    x_min -= x_delta * 0.5;
    x_max += x_delta * 0.5;
    y_min -= y_delta * 0.5;
    y_max += y_delta * 0.5;
    
    m_Points.push_back( Eigen::Vector2d( x_min, y_min ) );
    m_Points.push_back( Eigen::Vector2d( x_max, y_min ) );
    m_Points.push_back( Eigen::Vector2d( x_max, y_max ) );
    m_Points.push_back( Eigen::Vector2d( x_min, y_max ) );
    
    AddTriangle( PointSize, PointSize + 1, PointSize + 2 );
    AddTriangle( PointSize, PointSize + 2, PointSize + 3 );
    
    m_InitTrianglePointIndex.clear();
    
    for( unsigned long i = 0; i < 4; ++i ) {
        m_InitTrianglePointIndex.push_back( PointSize + i );
    }
}

void Delaunay::DeleteInitTriangle()
{
    std::set< unsigned long > PointIndex;
    
    for( auto index : m_InitTrianglePointIndex ) {
        PointIndex.insert( index );
    }
    
    m_Triangles.erase( std::remove_if( begin(m_Triangles), end(m_Triangles), [PointIndex]( TriangleData &tri ) {
        
        if( PointIndex.find( tri.index1 ) != PointIndex.end() )
            return true;
        
        if( PointIndex.find( tri.index2 ) != PointIndex.end() )
            return true;
        
        if( PointIndex.find( tri.index3 ) != PointIndex.end() )
            return true;
        
        return false;
        
    }), end( m_Triangles ) );
    
    for( auto index = m_InitTrianglePointIndex.rbegin(); index != m_InitTrianglePointIndex.rend(); ++index ) {
        m_Points.erase( m_Points.begin() + *index );
    }
    
}

void Delaunay::AddTriangle( unsigned long index1, unsigned long index2, unsigned long index3 )
{
    Eigen::Vector2d p1 = m_Points[index1];
    Eigen::Vector2d p2 = m_Points[index2];
    Eigen::Vector2d p3 = m_Points[index3];
    
    Eigen::MatrixXd A(2,2);
    Eigen::VectorXd b(2);
    
    A( 0, 0 ) = 2.0 * ( p2.x() - p1.x() );
    A( 0, 1 ) = 2.0 * ( p2.y() - p1.y() );
    A( 1, 0 ) = 2.0 * ( p3.x() - p1.x() );
    A( 1, 1 ) = 2.0 * ( p3.y() - p1.y() );
    
    b(0) = - p1.x() * p1.x() + p2.x() * p2.x() - p1.y() * p1.y() + p2.y() * p2.y();
    b(1) = - p1.x() * p1.x() + p3.x() * p3.x() - p1.y() * p1.y() + p3.y() * p3.y();
    
    Eigen::VectorXd x = A.fullPivLu().solve(b);

    TriangleData triangle;
    triangle.index1 = index1;
    triangle.index2 = index2;
    triangle.index3 = index3;
    triangle.center[0] = x[0];
    triangle.center[1] = x[1];
    triangle.radius = ( triangle.center - p1 ).norm();
    
    triangle.bounds[0] = triangle.center.x() - triangle.radius;
    triangle.bounds[1] = triangle.center.x() + triangle.radius;
    triangle.bounds[2] = triangle.center.y() - triangle.radius;
    triangle.bounds[3] = triangle.center.y() + triangle.radius;

    this->m_Triangles.push_back( triangle );
}

void Delaunay::Triangulation()
{
    CreateInitTriangle();
    
    std::vector< Eigen::Vector2d >::iterator itPoint;
    std::vector< EdgeData >::iterator itEdge1, itEdge2;
    unsigned long index, PointCount;
    bool check;
    
    PointCount = m_Points.size() - m_InitTrianglePointIndex.size();
    
    for( index = 0, itPoint = m_Points.begin(); index < PointCount; ++index, ++itPoint ) {
        
        std::vector< TriangleData > triangles;
        std::vector< EdgeData > edges;
        
        for( auto tri : m_Triangles ) {
            if( tri.IsInside( *itPoint ) ) {
                edges.push_back( EdgeData( tri.index1, tri.index2 ) );
                edges.push_back( EdgeData( tri.index2, tri.index3 ) );
                edges.push_back( EdgeData( tri.index3, tri.index1 ) );
            } else {
                triangles.push_back( tri );
            }
        }
        
        m_Triangles.resize( triangles.size() );
        std::copy( triangles.begin(), triangles.end(), m_Triangles.begin() );
        
        std::sort( edges.begin(), edges.end(), []( EdgeData a, EdgeData b ) {
            if( a.index1 < b.index1 ) {
                return true;
            } else if( a.index1 == b.index1 ) {
                if( a.index2 < b.index2 ) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        });
        
        for( itEdge1 = edges.begin(), itEdge2 = edges.begin() + 1, check = true; itEdge2 != edges.end(); ++itEdge1, ++itEdge2 ) {
            if( (*itEdge1).index1 == (*itEdge2).index1 && (*itEdge1).index2 == (*itEdge2).index2 ) {
                check = false;
            } else {
                if( check ) {
                    AddTriangle( (*itEdge1).index1, (*itEdge1).index2, index );
                }
                check = true;
            }
        }
        
        itEdge1 = edges.begin() + edges.size() - 2;
        itEdge2 = edges.begin() + edges.size() - 1;
        
        if( !( (*itEdge1).index1 == (*itEdge2).index1 && (*itEdge1).index2 == (*itEdge2).index2 ) ) {
            AddTriangle( (*itEdge2).index1, (*itEdge2).index2, index );
        }
        
    }
    
    DeleteInitTriangle();
    
}
