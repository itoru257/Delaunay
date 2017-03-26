/*************************************************
 * Copyright (c) 2017 Toru Ito
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 *************************************************/

#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <string>

#include <fstream>
#include <sstream>

#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram()
    :m_Program(0)
{
}

ShaderProgram::~ShaderProgram()
{
}

std::string shader_vertex =
R"(
#version 410

uniform mat4 DataMatrix;
in vec3 VertexPosition;

void main()
{
    gl_Position = DataMatrix * vec4(VertexPosition,1.0);
}
)";


std::string shader_fragment =
R"(
#version 410

uniform vec4 color = vec4(1.0, 0.0, 0.0, 1.0);
out vec4 output_color;

void main()
{
    output_color = color;
}
)";

bool ShaderProgram::InitProgram()
{
    if( !this->CreateProgram() )
        return false;
    
    if( !this->AddShader( shader_vertex, GL_VERTEX_SHADER ) )
        return false;
    
    if( !this->AddShader( shader_fragment, GL_FRAGMENT_SHADER ) )
        return false;
    
    if( !this->LinkProgram() )
        return false;
    
    if( !this->UseProgram() )
        return false;
    
    return true;
}

bool ShaderProgram::InitProgramFromFile( std::string fname_vertex, std::string fname_fragment )
{
    if( !this->CreateProgram() )
        return false;
    
    if( !this->AddShaderFromFile( fname_vertex, GL_VERTEX_SHADER ) )
        return false;
    
    if( !this->AddShaderFromFile( fname_fragment, GL_FRAGMENT_SHADER ) )
        return false;
    
    if( !this->LinkProgram() )
        return false;
    
    if( !this->UseProgram() )
        return false;
    
    return true;
}

bool ShaderProgram::CreateProgram()
{
    m_Program = glCreateProgram();
    
    if( m_Program == 0 ) {
        m_Log.push_back( "Error creating program object." );
        return false;
    }
    
    return true;
}

bool ShaderProgram::AddShader( std::string code, GLenum shaderType )
{
    // Create the shader object
    GLuint shader = glCreateShader( shaderType );
    
    if( shader == 0 ) {
        m_Log.push_back( "Error creating shader" );
        return false;
    }
    
    // Load the source code into the shader object
    const GLchar* codeArray[] = { code.c_str() };
    glShaderSource( shader, 1, codeArray, NULL );
    
    // Compile the shader
    glCompileShader( shader );
    
    // Check compilation status
    GLint result;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &result );
    
    if( GL_FALSE == result ) {
        m_Log.push_back( "shader compilation failed" );
        
        GLint logLen;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLen );
        
        if( logLen > 0 ) {
            char * log = new char[logLen];
            
            GLsizei written;
            glGetShaderInfoLog( shader, logLen, &written, log );
            
            std::string text;
            text = "Shader log: ";
            text += log;
            
            m_Log.push_back( text );
            
            delete [] log;
        }
        
        return false;
    }
    
    glAttachShader( m_Program, shader );
    glDeleteShader( shader );
    
    return true;
}

bool ShaderProgram::AddShaderFromFile( std::string file_name, GLenum shaderType )
{
    std::ifstream inFile( file_name.c_str(), std::ios::in );
    
    if( !inFile ) {
        std::string text;
        text = "Error opening file: ";
        text += file_name;
        
        m_Log.push_back( text );
        return false;
    }
    
    std::stringstream code;
    code << inFile.rdbuf();
    inFile.close();
    
    std::string codeStr( code.str() );
    
    return AddShader( codeStr, shaderType );
}

bool ShaderProgram::LinkProgram()
{
    // Link the program
    glLinkProgram( m_Program );
    
    // Check for successful linking
    GLint status;
    glGetProgramiv( m_Program, GL_LINK_STATUS, &status );
    
    if (GL_FALSE == status) {
        
        m_Log.push_back( "Failed to link shader program" );
        
        GLint logLen;
        glGetProgramiv( m_Program, GL_INFO_LOG_LENGTH, &logLen );
        
        if( logLen > 0 ) {
            char * log = new char[logLen];
            
            GLsizei written;
            glGetProgramInfoLog( m_Program, logLen, &written, log );
            
            std::string text;
            text = "Program log: ";
            text += log;
            m_Log.push_back( log );
            
            delete [] log;
        }
        
        return false;
        
    }
    
    return true;
}

bool ShaderProgram::UseProgram()
{
    if( m_Program == 0 )
        return false;
    
    glUseProgram( m_Program );
    
    return true;
}


bool ShaderProgram::DeleteProgram()
{
    if( m_Program == 0 )
        return false;
    
    glDeleteProgram( m_Program );
    
    return true;
}

void ShaderProgram::PrintLog()
{
    for( auto log : m_Log ) {
        fprintf( stderr, "%s\n", log.c_str() );
    }
}

void ShaderProgram::SetColor( float r, float g, float b )
{
    float color[4] = { r, g, b, 1.0f };
    
    GLint uniform = glGetUniformLocation( m_Program, "color" );
    glProgramUniform4fv( m_Program, uniform, 1, color );
}

void ShaderProgram::SetDataMatrix( Eigen::Vector3f min, Eigen::Vector3f max, Eigen::Vector3f trans )
{
    Eigen::Vector3f offset = ( ( max - min ) * 0.5 + min ) * - 1.0;
    float scale = 1.5f / ( max - min ).maxCoeff();
    
    Eigen::Affine3f matrix = Eigen::Affine3f::Identity();
    matrix.translate( trans );
    matrix.scale( Eigen::Vector3f( scale, scale, scale ) );
    matrix.translate( offset );
    
    GLint data_matrix = glGetUniformLocation( m_Program, "DataMatrix" );
    glUniformMatrix4fv( data_matrix, 1, GL_FALSE, matrix.matrix().data() );
}

void ShaderProgram::SetDataMatrix( ShapeData *data, Eigen::Vector3f trans )
{
    Eigen::Vector3f min = data->m_Vertex.colwise().minCoeff().cast<float>();
    Eigen::Vector3f max = data->m_Vertex.colwise().maxCoeff().cast<float>();
    
    Eigen::Vector3f offset = ( ( max - min ) * 0.5 + min ) * - 1.0;
    float scale = 1.5f / ( max - min ).maxCoeff();
    
    Eigen::Affine3f matrix = Eigen::Affine3f::Identity();
    matrix.translate( trans );
    matrix.scale( Eigen::Vector3f( scale, scale, scale ) );
    matrix.translate( offset );
    
    GLint data_matrix = glGetUniformLocation( m_Program, "DataMatrix" );
    glUniformMatrix4fv( data_matrix, 1, GL_FALSE, matrix.matrix().data() );
}

void ShaderProgram::SetDataMatrix( std::vector< ShapeData > *datas, Eigen::Vector3f trans )
{
    Eigen::MatrixXd mins = Eigen::MatrixXd::Zero( datas->size(), 3 );
    Eigen::MatrixXd maxs = Eigen::MatrixXd::Zero( datas->size(), 3 );
    size_t i;
    
    for( i = 0; i < datas->size(); ++i ) {
        mins.row(i) = datas->at(i).m_Vertex.colwise().minCoeff();
        maxs.row(i) = datas->at(i).m_Vertex.colwise().maxCoeff();
    }
        
    Eigen::Vector3f min = mins.colwise().minCoeff().cast<float>();
    Eigen::Vector3f max = maxs.colwise().maxCoeff().cast<float>();
    
    Eigen::Vector3f offset = ( ( max - min ) * 0.5 + min ) * - 1.0;
    float scale = 1.5f / ( max - min ).maxCoeff();
    
    Eigen::Affine3f matrix = Eigen::Affine3f::Identity();
    matrix.translate( trans );
    matrix.scale( Eigen::Vector3f( scale, scale, scale ) );
    matrix.translate( offset );
    
    GLint data_matrix = glGetUniformLocation( m_Program, "DataMatrix" );
    glUniformMatrix4fv( data_matrix, 1, GL_FALSE, matrix.matrix().data() );
}

