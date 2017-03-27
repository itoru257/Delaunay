//
//  main.cpp
//  Delaunay
//
//  Created by toru ito on 2017/03/22.
//  Copyright © 2017年 toru ito. All rights reserved.
//

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include "ShaderProgram.hpp"
#include "ShapeData.hpp"
#include "Delaunay.hpp"

int main(int argc, const char * argv[]) {
    
    GLFWwindow* window;
    
    // Initialize the library
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow( 500, 500, "Delaunay", NULL, NULL );
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    // Make the window's context current
    glfwMakeContextCurrent(window);
    
    glEnable(GL_DEPTH_TEST);
    
    std::vector< Eigen::Vector2d > PointList;
    std::vector< std::vector< unsigned int > > IndexList;
    
    double x, y;
    int k;
    
    srand( 2 );
    //srand((unsigned int)time(NULL));
    
    for( k = 0; k < 100; ++k ) {
        x = static_cast< double >( rand() ) / static_cast< double >( RAND_MAX );
        y = static_cast< double >( rand() ) / static_cast< double >( RAND_MAX );
        PointList.push_back( Eigen::Vector2d( x, y ) );
    }
    
    Delaunay delaunay;
    delaunay.SetPoint( &PointList );
    delaunay.Triangulation();
    delaunay.GetResult( &PointList, &IndexList );

    ShaderProgram triangleProgram, wireframeProgram;
    ShapeData triangle, wireframe;
    
    //triangle
    if( !triangleProgram.InitProgram() )
        return -1;
    
    triangle.SetIndex( IndexList );
    triangle.SetVertex( PointList );
    triangle.InitDraw();
    triangle.SetDrawMode( GL_TRIANGLES );
    
    triangleProgram.SetColor( 0.0f, 0.0f, 1.0f );
    triangleProgram.SetDataMatrix( &triangle, Eigen::Vector3f( 0.0f, 0.0f, 0.0f ) );
    
    //wireframe
    std::vector< std::vector< unsigned int > > WireframeIndexList;
    std::vector< unsigned int > WireframeIndexs;
    unsigned int i, j;
    
    WireframeIndexs.resize( 2 );
    
    for( auto indexs : IndexList ) {
        for( i = 0; i < 3; ++i ) {
            j = ( i + 1 ) % 3;
            WireframeIndexs[0] = indexs[i];
            WireframeIndexs[1] = indexs[j];
            WireframeIndexList.push_back( WireframeIndexs );
        }
    }
    
    if( !wireframeProgram.InitProgram() )
        return -1;
    
    wireframe.SetIndex( WireframeIndexList );
    wireframe.SetVertex( PointList );
    wireframe.InitDraw();
    wireframe.SetDrawMode( GL_LINES );
    
    wireframeProgram.SetColor( 1.0f, 1.0f, 1.0f );
    wireframeProgram.SetDataMatrix( &wireframe, Eigen::Vector3f( 0.0f, 0.0f, -0.1f ) );
    
    // Loop until the user closes the window
    while( !glfwWindowShouldClose( window ) ) {
        
        // Render
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        triangleProgram.UseProgram();
        triangle.Draw();
        
        wireframeProgram.UseProgram();
        wireframe.Draw();
        
        glBindVertexArray( 0 );
        
        // Swap front and back buffers
        glfwSwapBuffers( window );
        
        // Poll for and process events
        glfwPollEvents();
        
    }
    
    triangle.DeleteDraw();
    triangleProgram.DeleteProgram();
    
    wireframe.DeleteDraw();
    wireframeProgram.DeleteProgram();
    
    glfwTerminate();
    
    return 0;
}
