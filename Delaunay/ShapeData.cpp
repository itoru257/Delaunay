/*************************************************
 * Copyright (c) 2016 Toru Ito
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 *************************************************/

#include "ShapeData.hpp"

ShapeData::ShapeData()
    :m_DrawMode( GL_LINES )
    ,m_VertexBufferObject( 0 )
    ,m_IndexBufferObject( 0 )
    ,m_VertexArrayObject( 0 )
{
}

ShapeData::~ShapeData()
{
}

void ShapeData::ConvertListToMatrix( std::vector< std::vector< double > >& list, MatrixDouble& matrix )
{
    size_t m, n, i, j;
    
    m = list.size();
    n = list.front().size();
    
    matrix.resize( m, n );
    
    for( i = 0; i < m; ++i ) {
        for( j = 0; j < n; ++j ) {
            matrix( i, j ) = list[i][j];
        }
    }
}

void ShapeData::ConvertListToMatrix( std::vector< std::vector< unsigned int > >& list, MatrixUInt& matrix )
{
    size_t m, n, i, j;
    
    m = list.size();
    n = list.front().size();
    
    matrix.resize( m, n );
    
    for( i = 0; i < m; ++i ) {
        for( j = 0; j < n; ++j ) {
            matrix( i, j ) = list[i][j];
        }
    }
}

void ShapeData::SetIndex( std::vector< std::vector< unsigned int > >& list )
{
    ConvertListToMatrix( list, m_Index );
}

void ShapeData::SetVertex( std::vector< std::vector< double > >& list )
{
    ConvertListToMatrix( list, m_Vertex );
}

void ShapeData::SetVertex( std::vector< Eigen::Vector2d >& list )
{
    std::vector< std::vector< double > > vertexs;
    std::vector< double > vertex;
    
    vertex.resize( 3 );

    for( auto point : list ) {
        vertex[0] = point.x();
        vertex[1] = point.y();
        vertex[2] = 0.0;
        vertexs.push_back(vertex);
    }

    ConvertListToMatrix( vertexs, m_Vertex );
}

void ShapeData::SetPolyline( std::vector< std::vector< std::vector< Eigen::Vector2d > > > *pPointList )
{
    std::vector< std::vector< unsigned int > > indexs;
    std::vector< unsigned int > index;
    
    std::vector< std::vector< double > > vertexs;
    std::vector< double > vertex;
    
    index.resize( 2 );
    vertex.resize( 3 );
   
    for( auto group : *pPointList ) {
        for( auto item : group ) {
            for( auto it = item.begin(); it != item.end(); ++it ) {
                
                vertex[0] = (*it).x();
                vertex[1] = (*it).y();
                vertex[2] = 0.0;
                
                if( it != item.begin() ) {
                    index[0] = static_cast< unsigned int >( vertexs.size() - 1 );
                    index[1] = static_cast< unsigned int >( vertexs.size() );
                    indexs.push_back( index );
                }
                
                vertexs.push_back( vertex );
                
            }
        }
    }
    
    SetVertex( vertexs );
    SetIndex( indexs );
    SetDrawMode( GL_LINES );

}

void ShapeData::SetDrawMode( GLenum mode )
{
    m_DrawMode = mode;
}

void ShapeData::InitDraw()
{
    glGenBuffers( 1, &m_VertexBufferObject );
    
    glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferObject );
    glBufferData( GL_ARRAY_BUFFER, sizeof(double)*m_Vertex.size(), m_Vertex.data(), GL_STATIC_DRAW );
    
    glGenBuffers( 1, &m_IndexBufferObject );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_Index.size(), m_Index.data(), GL_STATIC_DRAW );
    
    glGenVertexArrays( 1, &m_VertexArrayObject );
    glBindVertexArray( m_VertexArrayObject );
    
    glEnableVertexAttribArray( 0 );
    
    glBindBuffer( GL_ARRAY_BUFFER, m_VertexBufferObject );
    glVertexAttribPointer( 0, (GLint)m_Vertex.cols(), GL_DOUBLE, GL_FALSE, 0, (GLubyte *)NULL );
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject );
    
    glBindVertexArray( 0 );
}

void ShapeData::Draw()
{
    glBindVertexArray( m_VertexArrayObject );
    glDrawElements( m_DrawMode, (GLsizei)m_Index.size(), GL_UNSIGNED_INT, 0 );
}

void ShapeData::DeleteDraw()
{
    if( m_VertexBufferObject != 0 ) {
        glDeleteBuffers( 1, &m_VertexBufferObject );
        m_VertexBufferObject = 0;
    }
    
    if( m_IndexBufferObject != 0 ) {
        glDeleteBuffers( 1, &m_IndexBufferObject );
        m_IndexBufferObject = 0;
    }
    
    if( m_VertexArrayObject != 0 ) {
        glDeleteVertexArrays( 1, &m_VertexArrayObject );
        m_VertexArrayObject = 0;
    }
}

