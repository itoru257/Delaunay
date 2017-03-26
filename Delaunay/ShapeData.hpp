/*************************************************
 * Copyright (c) 2017 Toru Ito
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 *************************************************/

#ifndef ShapeData_hpp
#define ShapeData_hpp

#include <stdio.h>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <OpenGL/gl3.h>

class ShapeData
{
public:
    typedef Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor >       MatrixDouble;
    typedef Eigen::Matrix< unsigned int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > MatrixUInt;

public:
    ShapeData();
    ~ShapeData();
    
    static void ConvertListToMatrix( std::vector< std::vector< double > >& list, MatrixDouble& matrix );
    static void ConvertListToMatrix( std::vector< std::vector< unsigned int > >& list, MatrixUInt& matrix );
    
    void SetPolyline( std::vector< std::vector< std::vector< Eigen::Vector2d > > > *pPointList );
    void SetVertex( std::vector< std::vector< double > >& list );
    void SetVertex( std::vector< Eigen::Vector2d >& list );
    void SetIndex( std::vector< std::vector< unsigned int > >& list );
    
    void SetDrawMode( GLenum mode );
    
    void InitDraw();
    void Draw();
    void DeleteDraw();
    
    MatrixDouble  m_Vertex;
    MatrixUInt    m_Index;
    
private:
    GLuint m_VertexBufferObject;
    GLuint m_IndexBufferObject;
    GLuint m_VertexArrayObject;
    
    GLenum m_DrawMode;
};

#endif /* ShapeData_hpp */
