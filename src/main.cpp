#include <TextureAndLightingPCH.h>
#include <Camera.h>
//to set up the vertex buffer object extension
#define BUFFER_OFFSET(offset) ((void*)(offset))

//set size of window
int sWindowWidth = 1300;
int sWindowHeight = 700;
int sWindowHandle = 0;

float OBJRotation = 0.0f;

std::clock_t PreviousTicks;//for compute time between frames
std::clock_t CurrentTicks;//same

Camera Camera1;

GLuint vaoSphere = 0;
GLuint TexturedDiffuseShaderProgram = 0;
GLuint SimpleShaderProgram = 0;
// Model, View, Projection matrix uniform variable in shader program.
GLint uniformMVP = -1;
GLint uniformModelMatrix = -1;
GLint uniformEyePosW = -1;

GLint uniformColor = -1;

// Light uniform variables.
GLint uniformLightPosW = -1;
GLint uniformLightColor = -1;
GLint uniformAmbient = -1;

// Material properties.
GLint uniformMaterialEmissive = -1;
GLint uniformMaterialDiffuse = -1;
GLint uniformMaterialSpecular = -1;
GLint uniformMaterialShininess =-1;

GLuint EarthTexture = 0;
GLuint SunTexture = 0;

void IdleGL();
void updateGL();
void KeyboardGL( unsigned char c, int x, int y );
void ReshapeGL( int w, int h );

int isAnimate = 0;  //for run animate

//open the shader file and checking
GLuint LoadShader( GLenum shaderType, const std::string& shaderFile )
{
    std::ifstream ifs;

    // Load the shader.
    ifs.open(shaderFile);

    std::string source( std::istreambuf_iterator<char>(ifs), (std::istreambuf_iterator<char>()) );
    ifs.close();

    // Create a shader object.
    GLuint shader = glCreateShader( shaderType );

    // Load the shader source for each shader object.
    const GLchar* sources[] = { source.c_str() };
    glShaderSource( shader, 1, sources, NULL );

    // Compile the shader.
    glCompileShader( shader );

    // Check for errors
    GLint compileStatus;
    //checking shader compilation
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compileStatus ); 
    if ( compileStatus != GL_TRUE )
    {
        GLint logLength;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );
        GLchar* infoLog = new GLchar[logLength];
        glGetShaderInfoLog( shader, logLength, NULL, infoLog );

        delete infoLog;
        return 0;
    }

    return shader;
}

GLuint CreateShaderProgram( std::vector<GLuint> shaders )
{
    // Create a shader program.
    GLuint program = glCreateProgram();

    // Attach the appropriate shader objects.
    for( GLuint shader: shaders )
    {
        glAttachShader( program, shader );
    }
    // Link the program
    glLinkProgram(program);
    // Check the link status.
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus );
    if ( linkStatus != GL_TRUE )
    {
        GLint logLength;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );
        GLchar* infoLog = new GLchar[logLength];

        glGetProgramInfoLog( program, logLength, NULL, infoLog );

        delete infoLog;
        return 0;
    }

    return program;
}

GLuint LoadTexture( const std::string& file )
{
    GLuint textureID = SOIL_load_OGL_texture( file.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS );//load texture
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );//modifying
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glBindTexture( GL_TEXTURE_2D, 0 );//A texture target that is used to bind to a 2D texture object.

    return textureID;
}

GLuint drawSphere( float radius, int slices, int stacks )
{
    using namespace glm;
    using namespace std;

    const float pi = 3.14;
    const float _2pi = 2.0f * pi;

    vector<vec3> positions;
    vector<vec3> normals;
    vector<vec2> textureCoords;

    for( int i = 0; i <= stacks; ++i )
    {
        float V = i / (float)stacks;
        float phi = V * pi;

        for ( int j = 0; j <= slices; ++j )
        {
            float U = j / (float)slices;
            float theta = U * _2pi;

            float X = cos(theta) * sin(phi);
            float Y = cos(phi);
            float Z = sin(theta) * sin(phi);

            positions.push_back( vec3( X, Y, Z) * radius );
            normals.push_back( vec3(X, Y, Z) );
            textureCoords.push_back( vec2(U, V) );
        }
    }

    // index buffer
    vector<GLuint> indicies;

    for( int i = 0; i < slices * stacks + slices; ++i )
    {
        indicies.push_back( i );
        indicies.push_back( i + slices + 1  );
        indicies.push_back( i + slices );

        indicies.push_back( i + slices + 1  );
        indicies.push_back( i );
        indicies.push_back( i + 1 );
    }

    GLuint vao;//vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    GLuint vbos[4];
    glGenBuffers( 4, vbos );

    glBindBuffer( GL_ARRAY_BUFFER, vbos[0] );
    glBufferData( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vbos[1] );
    glBufferData( GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), normals.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( 2 );

    glBindBuffer( GL_ARRAY_BUFFER, vbos[2] );
    glBufferData( GL_ARRAY_BUFFER, textureCoords.size() * sizeof(vec2), textureCoords.data(), GL_STATIC_DRAW );
    glVertexAttribPointer( 8, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( 8 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbos[3] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW );

    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    return vao;
}

int main( int argc, char* argv[] )
{
    PreviousTicks = std::clock();
	Camera1.SetPosition(glm::vec3(0, 0, 200));//200 units to z axis
	Camera1.SetRotation( glm::quat(1, 0, 0, 0) );
	// setup and initializing
	glutInit(&argc, argv);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);

	// Create an OpenGL 3.3 core forward compatible context.
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowPosition((iScreenWidth - sWindowWidth) / 2, (iScreenHeight - sWindowHeight) / 2);
	glutInitWindowSize(sWindowWidth, sWindowHeight);

	sWindowHandle = glutCreateWindow("Solar system project");

	// Register GLUT callbacks.
	glutIdleFunc(IdleGL);
	glutDisplayFunc(updateGL);
	glutKeyboardFunc(KeyboardGL);
	glutReshapeFunc(ReshapeGL);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // set back colour
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glewExperimental = GL_TRUE;
	glewInit();
//load of textures
    SunTexture = LoadTexture( "../data/Textures/sun.dds" );
    EarthTexture =   LoadTexture("../data/Textures/earth.dds");

    GLuint vertexShader =   LoadShader( GL_VERTEX_SHADER, "../data/Shaders/simpleShader.vert" );
    GLuint fragmentShader = LoadShader( GL_FRAGMENT_SHADER, "../data/Shaders/simpleShader.frag" );

    std::vector<GLuint> shaders;
    shaders.push_back(vertexShader);
    shaders.push_back(fragmentShader);

    SimpleShaderProgram = CreateShaderProgram( shaders );//creating program of shaders
    assert( SimpleShaderProgram );

   
    
    uniformColor = glGetUniformLocation( SimpleShaderProgram, "color" );
//loading of shaders
    vertexShader =   LoadShader( GL_VERTEX_SHADER, "../data/Shaders/texturedDiffuse.vert" );
    fragmentShader = LoadShader( GL_FRAGMENT_SHADER, "../data/Shaders/texturedDiffuse.frag" );

    shaders.clear();

    shaders.push_back(vertexShader);
    shaders.push_back(fragmentShader);
    TexturedDiffuseShaderProgram = CreateShaderProgram( shaders );
    assert( TexturedDiffuseShaderProgram );

    uniformMVP =         glGetUniformLocation( TexturedDiffuseShaderProgram, "ModelViewProjectionMatrix" );
    uniformModelMatrix = glGetUniformLocation( TexturedDiffuseShaderProgram, "ModelMatrix" );
    uniformEyePosW =     glGetUniformLocation( TexturedDiffuseShaderProgram, "EyePosW" );
    // Light properties.
    uniformLightPosW =  glGetUniformLocation( TexturedDiffuseShaderProgram, "LightPosW" );
    uniformLightColor = glGetUniformLocation( TexturedDiffuseShaderProgram, "LightColor" );
    // Global ambient.
    uniformAmbient = glGetUniformLocation( TexturedDiffuseShaderProgram, "Ambient" );

    // Material 
    uniformMaterialEmissive =  glGetUniformLocation( TexturedDiffuseShaderProgram, "MaterialEmissive" );
    uniformMaterialDiffuse =   glGetUniformLocation( TexturedDiffuseShaderProgram, "MaterialDiffuse" );
    uniformMaterialSpecular =  glGetUniformLocation( TexturedDiffuseShaderProgram, "MaterialSpecular" );
    uniformMaterialShininess = glGetUniformLocation( TexturedDiffuseShaderProgram, "MaterialShininess" );

    glutMainLoop();
}

void ReshapeGL( int w, int h )
{
    if ( h == 0 )  { h = 1;}
    sWindowWidth = w;
    sWindowHeight = h;

	Camera1.SetViewport( 0, 0, w, h );
	Camera1.SetProjectionRH( 25.0f, w/(float)h, 0.1f, 300.0f );
    glutPostRedisplay();
}

void updateGL()
{
    
    const int numIndicies = ( 16*16 ) * 6;
    if ( vaoSphere == 0 )
    {
         vaoSphere = drawSphere( 2, 16, 16);
    }

    const glm::vec4 white(1.0f);//for setting un variables to sh
    const glm::vec4 black(0.7f);//for setting un variables to sh
    const glm::vec4 ambient( 0.1f, 0.1f, 0.1f, 1.0f );//for setting un variables to sh

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vaoSphere);

    // earth
	glBindTexture(GL_TEXTURE_2D, EarthTexture);
	glUseProgram(TexturedDiffuseShaderProgram);
    glm::mat4 modelMatrix = glm::rotate( glm::radians(OBJRotation), glm::vec3(0,-1,0) ) * glm::translate(glm::vec3(60,0,0));
    glm::mat4 mvp = Camera1.GetProjectionMatrix() * Camera1.GetViewMatrix() * modelMatrix;
    GLuint uniformMVP = glGetUniformLocation( SimpleShaderProgram, "MVP" );
	glUniformMatrix4fv( uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp) );
    glDrawElements( GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0) );
    
    // sun
    glBindTexture( GL_TEXTURE_2D, SunTexture );
    glUseProgram( TexturedDiffuseShaderProgram );

    modelMatrix = glm::rotate( glm::radians(OBJRotation), glm::vec3(0,1,0) ) * glm::scale(glm::vec3(10.756f) );
    glm::vec4 eyePosW = glm::vec4(Camera1.GetPosition(), 1 );
    mvp = Camera1.GetProjectionMatrix() * Camera1.GetViewMatrix() * modelMatrix;

    // Set the light position to the position of the Sun.
    glUniform4fv(uniformLightPosW, 1, glm::value_ptr(modelMatrix[3]));
    glUniform4fv(uniformLightColor, 1, glm::value_ptr(white));
    glUniform4fv(uniformAmbient, 1, glm::value_ptr(ambient));

    glUniformMatrix4fv( uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp) );
    glUniformMatrix4fv( uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix) );
    glUniform4fv( uniformEyePosW, 1, glm::value_ptr( eyePosW ) );

    // Material properties
    glUniform4fv( uniformMaterialEmissive, 1, glm::value_ptr(black) );
    glUniform4fv( uniformMaterialDiffuse, 1, glm::value_ptr(white) );
    glUniform4fv( uniformMaterialSpecular, 1, glm::value_ptr(white) );
    glUniform1f( uniformMaterialShininess, 1.0f ); 

    glDrawElements( GL_TRIANGLES, numIndicies, GL_UNSIGNED_INT, BUFFER_OFFSET(0) );

    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture( GL_TEXTURE_2D, 0 );
     glutSwapBuffers();
}

void IdleGL()
{
    CurrentTicks = std::clock();
    float deltaTicks = (float)( CurrentTicks - PreviousTicks );
    PreviousTicks = CurrentTicks;

    float fDeltaTime = deltaTicks / (float)CLOCKS_PER_SEC;

	if (isAnimate) { 
		OBJRotation += 35 * fDeltaTime;
		OBJRotation = fmod(OBJRotation, 360.0f);
	}

    glutPostRedisplay();
}

void KeyboardGL( unsigned char c, int x, int y )
{
    switch ( c )
    {
	case ' ':
		if (isAnimate) isAnimate = 0; else { isAnimate = 1; }
		break;
    }
}