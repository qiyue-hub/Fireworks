#include <stdlib.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <random>
#include <time.h>
#include <irrKlang.h>

using namespace std;

#define PI 3.14159265
#define BMP_Header_Length 54

// Window size
int width = 800;
int height = 600;

// Firework properties
float fireworkPosX = 0.0f;
float fireworkPosY = -0.8f;
float fireworkSize = 10.0f;

const int maxFirework = 7; // max number of existing firework (only number of firework < maxFirework, new firework can be generated)
const int maxParticle = 200; // the numbber of particle in a firework at the beginning
const int maxInterval = 0.5f;
int numFirework = 0; // number of existing firework
bool explosion[maxFirework], explosion2[maxFirework], noexplosion2[maxFirework];
int numParticle[maxFirework];
float posFirework[maxFirework], rgb[3][maxFirework], posFire[maxFirework]; // number of visible points of each firework, position of exposion

// For Look at
static GLdouble eyex = 0.0, eyey = 0.0, eyez = 5.0, centerx = 0.0, centery = 0.0, centerz = 0.0, upx = 0.0, upy = 1.0, upz = 0.0; //lookat参数

// Tree variables
GLfloat treeHeight = 1.5f;
GLfloat treeWidth = 0.2f;
GLfloat treeTrunkHeight = 0.6f;
GLfloat treeTrunkWidth = 0.1f;
GLfloat treeSwayAngle = 0.0f;
GLfloat treeSwaySpeed = 1.0f;

// Puppy variables
GLfloat puppyPositionX = 0.5f;
GLfloat puppyPositionZ = 3.2f;
GLfloat puppySpeedX = 0.0003f, puppySpeedZ = 0.0002f;
bool puppyRunning = true;


clock_t srt, ed;

GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };

GLuint texGround;
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice(); //prob error: 1. download 32 bit; 2.


// Particle structure
struct Particle {
    float posX;
    float posY;
    float posZ;
    float speedX;
    float speedY;
    float speedZ;
    float alpha;

    Particle(float x, float y, float z, float sx, float sy, float sz, float al)
        : posX(x), posY(y), posZ(z), speedX(sx), speedY(sy), speedZ(sz), alpha(al) {}
};

vector<Particle> particles;  // Vector to store particles

// Function to draw a firework explosion
void drawFirework() {

    
    for (int i = 0; i < particles.size() / maxParticle; i++) {

        for (int j = i * maxParticle; j < (i + 1) * maxParticle; j++) {
            glBegin(GL_POINTS);
            GLfloat sun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的环境光颜色，为0
            GLfloat sun_mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的漫反射光颜色，为0
            GLfloat sun_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的镜面反射光颜色，为0
            GLfloat sun_mat_emission[] = { rgb[0][i], rgb[1][i], rgb[2][i], 1.0f };   //定义材质的辐射广颜色
            GLfloat sun_mat_shininess = 0.0f;
            glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
            glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
            glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
            glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);

            Particle& particle = particles[j];

            glColor4f(rgb[0][i], rgb[1][i], rgb[2][i], particle.alpha);
            glVertex3f(particle.posX, particle.posY, particle.posZ);
            glEnd();
        }
    }
    

}

// Function to update the firework particles
void updateParticles() {
    for (int i = 0; i < particles.size() / maxParticle; i++) { // i-th firework

        for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) { //j-th particle


            Particle& particle = particles[j];
            if (!explosion[i]) {
                particle.posX += particle.speedX;
                particle.posY += particle.speedY;
                if (particle.alpha > 0)
                    particle.alpha -= 0.002f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            }
            else {
                particle.posX += particle.speedX;
                particle.posY += particle.speedY;
                particle.posZ += particle.speedZ;
                if (particle.alpha > 0) {
                    if (explosion2[i]) {
                        if (particle.alpha > 0.5)
                            particle.alpha -= 0.001f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                        else
                            particle.alpha -= 0.01f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    }
                    else 
                        particle.alpha -= 0.0007f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                }
            }

            if (particle.alpha <= 0.0f && particle.alpha > -0.5f) {
                numParticle[i]--;
                particle.alpha = -1.0f;
                /*particles.erase(particles.begin() + j);
                j--;*/
            }
        }

        if (explosion[i] && numParticle[i] < maxParticle && !noexplosion2[i] && !explosion2[i]) {
            float if_ex = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            if (if_ex > 0.5) { // second explosion
                explosion2[i] = true;
                float size = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f + 0.2f;
                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j += maxParticle / 8) {
                    int tmp_id = (j - i * maxParticle) / (maxParticle / 8);
                    for (int k = j; k < j + maxParticle / 8; k++) {

                        Particle& particle = particles[k];
                        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;
                        float rho = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0001f;
                        float speedX = rho * cos(angle);
                        float speedY = rho * sin(angle) - 0.00004f;
                        float speedZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f - 0.0001f; speedZ = speedZ * (1 - rho * rho);
                        particle.speedX = speedX; particle.speedY = speedY; particle.speedZ = speedZ;
                        particle.alpha = 1.0f;

                        particle.posX = posFire[i] + size * cos(tmp_id * PI / 4);
                        particle.posY = posFirework[i] + size * sin(tmp_id * PI / 4);
                        particle.posZ = 0.0f;
                    }
                }
                numParticle[i] = maxParticle;
                for (int c = 0; c < 2; c++) {
                    rgb[c][i] *= 2;
                    if (rgb[c][i] > 1.0f) rgb[c][i] = 1.0f;
                }
                //rgb[0][i] = 1.0f; rgb[1][i] = 1.0f; rgb[2][i] = 1.0f;
                for (int k = 0; k < 3; k++)
                    if (globalAmbient[k] < 0.15f)
                        globalAmbient[k] = 0.15f;
                SoundEngine->play2D("second_short.mp3", GL_FALSE);
            }
            else noexplosion2[i] = true;
        }

        if (numParticle[i] == 0) {
            if (explosion[i]) {  // exposion finished

                if (explosion2[i] || noexplosion2[i]) { // second explosion finished or no need to second explosion
                    particles.erase(particles.begin() + i * maxParticle, particles.begin() + (i + 1) * maxParticle); // debug: 从begin开始erase的
                    for (int k = i + 1; k < numFirework; k++) {
                        numParticle[k - 1] = numParticle[k];
                        posFirework[k - 1] = posFirework[k];
                        posFire[k - 1] = posFire[k];
                        explosion[k - 1] = explosion[k];
                        explosion2[k - 1] = explosion2[k];
                        noexplosion2[k - 1] = noexplosion2[k];
                        rgb[0][k - 1] = rgb[0][k]; rgb[1][k - 1] = rgb[1][k]; rgb[2][k - 1] = rgb[2][k];
                    }
                    numFirework--;
                    if (numFirework >= 0 && numFirework < maxFirework) {
                        numParticle[numFirework] = 0;
                        posFirework[numFirework] = 0;
                        posFire[numFirework] = 0;
                        explosion[numFirework] = false;
                        explosion2[numFirework] = false;
                        noexplosion2[numFirework] = false;
                        rgb[0][numFirework] = -1.0f; rgb[1][numFirework] = -1.0f; rgb[2][numFirework] = -1.0f;
                    }
                    else cout << "error in after explosion finished" << endl;
                }

                else { // second explosion
                    cout << "error in explosion2" << endl;
                }
                
            }
                
            else { // not exposion yet
                explosion[i] = true;
                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) {
                    Particle& particle = particles[j];
                    float prob = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    if (particle.posY > posFirework[i] && prob > 0.97f) 
                        posFirework[i] = particle.posY; // the highest as the explosion pos at 50 prob
                }
                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) {
                    Particle& particle = particles[j];
                    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;
                    float rho = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f;
                    float speedX = rho * cos(angle);
                    float speedY = rho * sin(angle);
                    float speedZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0004f - 0.0002f; speedZ = speedZ * (1 - rho * rho);
                    particle.speedX = speedX; particle.speedY = speedY; particle.speedZ = speedZ;
                    particle.alpha = 0.4f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.6f; particle.posY = posFirework[i];

                }
                numParticle[i] = maxParticle;
                for (int k = 0; k < 3; k++)
                    globalAmbient[k] = 0.2f;

                SoundEngine->play2D("explosion.mp3", GL_FALSE);
            }
        }

    }
}

// Function to generate firework particles
void generateParticles(float posX, float posY) {
    for (int j = 0; j < maxParticle; j++) {
        //float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
        //float speedY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f + 0.0002f;
        float speedY = log(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) / 1.2 * 0.001 + 0.002f;
        //cout << speedY << '\n';
        if (speedY < 0.0015) 
            speedY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0015f;
        float speedX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.00005f - 0.000025f;
        float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        particles.push_back(Particle(posX, posY, 0.0f, speedX, speedY, 0.0f, alpha));
    }
    numParticle[numFirework] = maxParticle;
    explosion[numFirework] = false;
    explosion2[numFirework] = false;
    noexplosion2[numFirework] = false;
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    if (double(r) + double(g) + double(b) < 0.33) {
        r = 2 * r + 0.3; g = 2 * g + 0.3; b = 2 * b + 0.3; // avoid too dark to see
    }

    rgb[0][numFirework] = r; rgb[1][numFirework] = g; rgb[2][numFirework] = b;
    posFire[numFirework] = posX;
    numFirework++;

    SoundEngine->play2D("fire.wav", GL_FALSE);

}

// Function to update the puppy's position and animation
void updatePuppy() {
    if (puppyRunning) {
        puppyPositionX += puppySpeedX;
        puppyPositionZ += puppySpeedZ;

        puppySpeedX = puppySpeedX * (0.1f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.9f);
        //puppySpeedX = (puppySpeedX + 0.0003f) / 2.0f;
        if (puppySpeedX > 0.002f) puppySpeedX = 0.002f;
        else if (puppySpeedX < -0.002f) puppySpeedX = -0.002f;
        else if (puppySpeedX > -0.00005f && puppySpeedX < 0.0f)  puppySpeedX = -0.0003f;
        else if (puppySpeedX < 0.00005f && puppySpeedX > 0.0f)  puppySpeedX = 0.0003f;

        puppySpeedZ = puppySpeedZ * (0.1f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.9f);
        //puppySpeedZ = (puppySpeedX + 0.0002f) / 2.0f;
        if (puppySpeedZ > 0.001f) puppySpeedZ = 0.0005f;
        else if (puppySpeedZ < -0.001f) puppySpeedZ = -0.0005f;
        else if (puppySpeedZ > -0.00005f && puppySpeedZ < 0.0f)  puppySpeedZ = -0.0003f;
        else if (puppySpeedZ < 0.00005f && puppySpeedZ > 0.0f)  puppySpeedZ = 0.0003f;


        // Wrap the puppy around to the left side of the screen
        if (puppyPositionX > 2.0f || puppyPositionX < -2.0f)
            puppyPositionX = -puppyPositionX;
        if (puppyPositionZ > 4.0f or puppyPositionZ < 0.5f)
            puppyPositionZ = 4.5f - puppyPositionZ;
    }
}

// Function to update the firework position and size
void update() {
    //cout << "in update" << endl;

    ed = clock();

    float prop = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    if (numFirework < maxFirework) {

        if (numFirework == 0 || (float((ed - srt) / CLOCKS_PER_SEC) > maxInterval && prop > 0.3)) {
            // Randomly generate a new firework position
            fireworkPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f - 2.0f;
            fireworkPosY = -0.5f;
            //fireworkSize = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f + 5.0f;

            // Generate firework particles
            generateParticles(fireworkPosX, fireworkPosY);

            srt = clock();
        }
        else if (float((ed - srt) / CLOCKS_PER_SEC) > maxInterval)
            srt = clock();
    }


    updateParticles();
    updatePuppy();


    glutPostRedisplay();
}

// Function to draw a brown wooden bench
void drawBench() {
    // Draw the seat
    glColor3f(0.612f, 0.42f, 0.188f);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 0.0f);
    glScalef(0.6f, 0.05f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw the legs
    glColor3f(0.435f, 0.29f, 0.184f);
    glPushMatrix();
    glTranslatef(-0.2f, -0.3f, -0.1f);
    glScalef(0.05f, 0.2f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.2f, -0.3f, -0.1f);
    glScalef(0.05f, 0.2f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.2f, -0.3f, 0.1f);
    glScalef(0.05f, 0.2f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.2f, -0.3f, 0.1f);
    glScalef(0.05f, 0.2f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// 函数power_of_two用于判断一个整数是不是2的整数次幂
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;
}

GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint last_texture_ID = 0, texture_ID = 0;
    FILE* pFile;
    errno_t err;

    // 打开文件，如果失败，返回
    if ((err = fopen_s(&pFile, file_name, "rb")) != 0)
    {
        printf("无法打开此文件\n");            //如果打不开，就输出打不开
        exit(0);                               //终止程序
    }

    // 读取文件中图象的宽度和高度
    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // 根据总像素字节数分配内存
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    // 读取像素数据
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
    // 若图像宽高超过了OpenGL规定的最大值，也缩放
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if (!power_of_two(width)
            || !power_of_two(height)
            || width > max
            || height > max)
        {
            const GLint new_width = 256;
            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // 计算每行需要的字节数和总字节数
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // 分配内存
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // 进行像素缩放
            gluScaleImage(GL_RGB,
                width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
            free(pixels);
            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // 分配一个新的纹理编号
    glGenTextures(1, &texture_ID);
    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 绑定新的纹理，载入纹理并设置纹理参数
    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
    GLint lastTextureID = last_texture_ID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
    free(pixels);
    return texture_ID;
}

// Function to draw a 3D tree
void drawTree()
{
    // Draw the tree trunk
    glPushMatrix();
    glColor3f(0.6f, 0.3f, 0.1f);
    glScalef(treeTrunkWidth, treeTrunkHeight, treeTrunkWidth);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw the tree leaves
    glPushMatrix();
    glColor3f(0.0f, 0.6f, 0.0f);
    glTranslatef(0.0f, treeTrunkHeight / 2.0f, 0.0f);
    glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    //glScalef(treeWidth, treeHeight, treeWidth);
    glutSolidCone(0.35, 0.25, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.0f, 0.6f, 0.0f);
    glTranslatef(0.0f, treeTrunkHeight / 2.0f + 0.15f, 0.0f);
    glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    //glScalef(treeWidth, treeHeight, treeWidth);
    glutSolidCone(0.3, 0.2, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.0f, 0.6f, 0.0f);
    glTranslatef(0.0f, treeTrunkHeight / 2.0f + 0.3f, 0.0f);
    glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    //glScalef(treeWidth, treeHeight, treeWidth);
    glutSolidCone(0.2, 0.2, 8, 8);
    glPopMatrix();
}

// Function to update the tree sway motion
void updateTreeSway()
{
    treeSwayAngle += treeSwaySpeed;

    if (treeSwayAngle > 360.0f)
        treeSwayAngle -= 360.0f;
}


// Function to draw a 3D puppy
void drawPuppy()
{
    // Draw the puppy's body
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    glTranslatef(0.0f, 0.4f, 0.0f);
    glScalef(1.2f, 0.8f, 0.8f);
    glutSolidSphere(0.28, 20, 20);
    glPopMatrix();

    // Draw the puppy's head
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    glTranslatef(-0.3f, 0.55f, 0.0f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.25f, 0.3f, 0.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw the puppy's ears
    glPushMatrix();
    glColor3f(1.0f, 0.8f, 0.8f);  // Light pink color for the ears
    glTranslatef(-0.3f, 0.75f, -0.2f);
    glScalef(0.02f, 0.18f, 0.14f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 0.8f, 0.8f);  // Light pink color for the ears
    glTranslatef(-0.3f, 0.75f, 0.2f);
    glScalef(0.02f, 0.18f, 0.14f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Draw the puppy's legs
    glPushMatrix();
    glColor3f(0.7f, 0.4f, 0.1f);  // White color
    glTranslatef(0.25f, 0.15f, 0.1f);
    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.3f, 0.07f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.7f, 0.4f, 0.1f);  // White color
    glTranslatef(0.25f, 0.15f, -0.1f);
    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.3f, 0.07f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.7f, 0.4f, 0.1f);  // White color
    glTranslatef(-0.25f, 0.15f, 0.1f);
    glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.3f, 0.07f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.7f, 0.4f, 0.1f);  // White color
    glTranslatef(-0.25f, 0.15f, -0.1f);
    glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.3f, 0.07f);
    glutSolidCube(1.0);
    glPopMatrix();

    //// Set the puppy's orientation based on its movement
    //if (puppyPositionX < 0.0f) {
    //    glTranslatef(puppyPositionX, puppyPositionY, 0.0f);
    //    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    //}
    //else if (puppyPositionX > 0.0f) {
    //    glTranslatef(puppyPositionX, puppyPositionY, 0.0f);
    //    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    //}
    //else {
    //    glTranslatef(puppyPositionX, puppyPositionY, 0.0f);
    //}
}


// Function to display the scene
void display() {
    //cout << "in display" << endl;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); // 打开混合
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 基于源象素alpha通道值的半透明混合函数

    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);

    // Adjust the overall ambient light
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    for (int k = 0; k < 3; k++)
        if (globalAmbient[k] > 0.1f)
            globalAmbient[k] -= 0.0001f;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);


    // Set up spotlight properties
    GLfloat lightDirection[] = { 0.2f, 0.4f, -5.0f, 1.0f };
    GLfloat lightExponent = 5.0f;
    GLfloat lightCutoff = 10.0f;
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, lightExponent);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, lightCutoff);
    // Enable and set up a secondary light source for the lamp
    glEnable(GL_LIGHT0);
    GLfloat lightPosition[] = { 0.4f, -0.3f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);


    glEnable(GL_TEXTURE_2D);    // 启用纹理
    //"D://code//VS code//ConsoleApplication1-v4.0//grass.bmp"

    // Draw the sky
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    // Draw the park ground
    //glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
    ///*glBegin(GL_QUADS);
    //glVertex3f(-2.0f, -0.5f, 0.0f);
    //glVertex3f(2.0f, -0.5f, 0.0f);
    //glVertex3f(2.0f, -0.5f, 4.0f);
    //glVertex3f(-2.0f, -0.5f, 4.0f);
    //glEnd();*/
    //glPushMatrix();
    //glTranslatef(-0.0f, -1.0f, 2.0f);
    //glScalef(4.0f, 1.0f, 4.0f);
    //glutSolidCube(1.0);
    //glPopMatrix();

    glEnable(GL_TEXTURE_2D);    //草地纹理
    glBindTexture(GL_TEXTURE_2D, texGround);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 0.0f);
    glTexCoord2f(0.0f, 5.0f); glVertex3f(2.0f, -0.5f, 0.0f);
    glTexCoord2f(5.0f, 5.0f); glVertex3f(2.0f, -0.5f, 4.0f);
    glTexCoord2f(5.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 4.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Draw the street lamp
    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.95f, 0.6f, 1.0f);
    glRotatef(90, 1, 0, 0);
    glutSolidCone(0.1, 0.1, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.95f, -0.0f, 1.0f);
    glScalef(0.02f, 1.0f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Set up spotlight properties
    GLfloat spotDirection[] = { 0.0f, -1.0f, 0.0f };
    GLfloat spotExponent = 15.0f;
    GLfloat spotCutoff = 45.0f;
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
    // Enable and set up a secondary light source for the lamp
    glEnable(GL_LIGHT1);
    GLfloat lampPosition[] = { -0.95f, 0.9f, 1.0f, 1.0f };
    GLfloat lampAmbient[] = { 1.0f, 0.95f, 0.5f, 1.0f };
    GLfloat lampDiffuse[] = { 1.0f, 0.95f, 0.5f, 1.0f };
    GLfloat lampSpecular[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lampPosition);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lampAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lampDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lampSpecular);


    drawFirework();

    
    GLfloat earth_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的环境光颜色，骗蓝色
    GLfloat earth_mat_diffuse[] = { 0.0f, 0.0f, 0.5f, 1.0f };  //定义材质的漫反射光颜色，偏蓝色
    GLfloat earth_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的镜面反射光颜色，红色
    GLfloat earth_mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的辐射光颜色，为0
    GLfloat earth_mat_shininess = 0.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, earth_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, earth_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, earth_mat_specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, earth_mat_emission);
    glMaterialf(GL_FRONT, GL_SHININESS, earth_mat_shininess);

    // Draw the bench
    glPushMatrix();
    glTranslatef(-0.5f, -0.1f, 1.0f);
    drawBench();
    glPopMatrix();

    // Draw the trees
    glPushMatrix();
    glTranslatef(0.5f, -0.5f, 2.6f);
    //glRotatef(treeSwayAngle, 0.0f, 1.0f, 0.0f);
    drawTree();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.9f, -0.5f, 2.3f);
    //glRotatef(-treeSwayAngle, 0.0f, 1.0f, 0.0f);
    drawTree();
    glPopMatrix();

    // Draw the puppy
    glPushMatrix();
    glTranslatef(puppyPositionX,-0.5f, puppyPositionZ);
    if (puppySpeedX > 0)
        glRotatef(180.0, 0.0, 1.0, 0.0);
    glScalef(0.3f, 0.3f, 0.3f);
    drawPuppy();
    glPopMatrix();

    glutSwapBuffers();
}

// 键盘控制视角转换

GLuint window;
GLuint View1;

void NormalKeys(unsigned char key, int x, int y) {
    switch (key) {
    case 'w'://W前进
        eyez -= 0.2f;
        glutPostRedisplay();
        break;
    case 's'://S后退
        eyez += 0.2f;
        glutPostRedisplay();
        break;
    case 'a'://A左行
        eyex -= 0.2f;
        centerx -= 0.2f;
        glutPostRedisplay();
        break;
    case 'd'://D右行
        eyex += 0.2f;
        centerx += 0.2f;
        glutPostRedisplay();
        break;
    case 'i'://I抬头
        upy = 0.1f;
        upz -= 0.4f;
        eyey += 0.002f;
        centery += 0.5f;
        glutPostRedisplay();
        break;
    case 'k'://K低头
        upy = 0.1f;
        upz += 0.4f;
        eyey -= 0.02f;
        centery -= 0.5f;
        glutPostRedisplay();
        break;
    case 'j'://J左转头
        centerx -= 0.2f;
        glutPostRedisplay();
        break;
    case 'l'://L右转头
        centerx += 0.2f;
        glutPostRedisplay();
        break;

    case 'x'://puppy left-right
        puppySpeedX = -puppySpeedX;
        glutPostRedisplay();
        break;
    case 'z'://puppy up-down
        puppySpeedZ = -puppySpeedZ;
        glutPostRedisplay();
        break;
    case 27://esc退出
        exit(0);
        break;
    case 32://空格复原
        eyex = 0.0;
        eyey = 0.0;
        eyez = 5.0;
        centerx = 0.0;
        centery = 0.0;
        centerz = 0.0;
        upx = 0.0; 
        upy = 1.0;
        upz = 0.0;
        puppySpeedX = 0.0003f;
        puppySpeedZ = 0.0002f;
        glutPostRedisplay();
        break;
    }
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Firework Show in the Park");
    // glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    texGround = load_texture("grass.bmp");

    //initLight();

    glutKeyboardFunc(NormalKeys);
    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutMainLoop();

    return 0;
}



// version 3, with grass texture, light, second explosion

//#include <stdlib.h>
//#include <GL/glut.h>
//#include <cmath>
//#include <vector>
//#include <iostream>
//#include <random>
//#include <time.h>
//#include <irrKlang.h>
//
//using namespace std;
//
//#define PI 3.14159265
//#define BMP_Header_Length 54
//
//// Window size
//int width = 800;
//int height = 600;
//
//// Firework properties
//float fireworkPosX = 0.0f;
//float fireworkPosY = -0.8f;
//float fireworkSize = 10.0f;
//
//const int maxFirework = 7; // max number of existing firework (only number of firework < maxFirework, new firework can be generated)
//const int maxParticle = 200; // the numbber of particle in a firework at the beginning
//const int maxInterval = 0.5f;
//int numFirework = 0; // number of existing firework
//bool explosion[maxFirework], explosion2[maxFirework], noexplosion2[maxFirework];
//int numParticle[maxFirework];
//float posFirework[maxFirework], rgb[3][maxFirework], posFire[maxFirework]; // number of visible points of each firework, position of exposion
//
//clock_t srt, ed;
//
//GLuint texGround;
//irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice(); //prob error: 1. download 32 bit; 2.
//
//
//// Particle structure
//struct Particle {
//    float posX;
//    float posY;
//    float posZ;
//    float speedX;
//    float speedY;
//    float speedZ;
//    float alpha;
//
//    Particle(float x, float y, float z, float sx, float sy, float sz, float al)
//        : posX(x), posY(y), posZ(z), speedX(sx), speedY(sy), speedZ(sz), alpha(al) {}
//};
//
//vector<Particle> particles;  // Vector to store particles
//
//// Function to draw a firework explosion
//void drawFirework() {
//
//
//    for (int i = 0; i < particles.size() / maxParticle; i++) {
//
//        for (int j = i * maxParticle; j < (i + 1) * maxParticle; j++) {
//            glBegin(GL_POINTS);
//            GLfloat sun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的环境光颜色，为0
//            GLfloat sun_mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的漫反射光颜色，为0
//            GLfloat sun_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的镜面反射光颜色，为0
//            GLfloat sun_mat_emission[] = { rgb[0][i], rgb[1][i], rgb[2][i], 1.0f };   //定义材质的辐射广颜色
//            GLfloat sun_mat_shininess = 0.0f;
//            glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
//            glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
//            glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
//            glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
//            glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
//
//            Particle& particle = particles[j];
//
//            glColor4f(rgb[0][i], rgb[1][i], rgb[2][i], particle.alpha);
//            glVertex3f(particle.posX, particle.posY, particle.posZ);
//            glEnd();
//        }
//    }
//
//
//}
//
//// Function to update the firework particles
//void updateParticles() {
//    for (int i = 0; i < particles.size() / maxParticle; i++) { // i-th firework
//
//        for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) { //j-th particle
//
//
//            Particle& particle = particles[j];
//            if (!explosion[i]) {
//                particle.posX += particle.speedX;
//                particle.posY += particle.speedY;
//                if (particle.alpha > 0)
//                    particle.alpha -= 0.002f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//            }
//            else {
//                particle.posX += particle.speedX;
//                particle.posY += particle.speedY;
//                particle.posZ += particle.speedZ;
//                if (particle.alpha > 0) {
//                    if (explosion2[i]) {
//                        if (particle.alpha > 0.5)
//                            particle.alpha -= 0.001f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//                        else
//                            particle.alpha -= 0.01f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//                    }
//                    else
//                        particle.alpha -= 0.0007f * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//                }
//            }
//
//            if (particle.alpha <= 0.0f && particle.alpha > -0.5f) {
//                numParticle[i]--;
//                particle.alpha = -1.0f;
//                /*particles.erase(particles.begin() + j);
//                j--;*/
//            }
//        }
//
//        if (explosion[i] && numParticle[i] < maxParticle && !noexplosion2[i] && !explosion2[i]) {
//            float if_ex = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//            if (if_ex > 0.5) { // second explosion
//                explosion2[i] = true;
//                float size = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f + 0.2f;
//                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j += maxParticle / 8) {
//                    int tmp_id = (j - i * maxParticle) / (maxParticle / 8);
//                    for (int k = j; k < j + maxParticle / 8; k++) {
//
//                        Particle& particle = particles[k];
//                        float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;
//                        float rho = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0001f;
//                        float speedX = rho * cos(angle);
//                        float speedY = rho * sin(angle) - 0.00004f;
//                        float speedZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f - 0.0001f; speedZ = speedZ * (1 - rho * rho);
//                        particle.speedX = speedX; particle.speedY = speedY; particle.speedZ = speedZ;
//                        particle.alpha = 1.0f;
//
//                        particle.posX = posFire[i] + size * cos(tmp_id * PI / 4);
//                        particle.posY = posFirework[i] + size * sin(tmp_id * PI / 4);
//                        particle.posZ = 0.0f;
//                    }
//                }
//                numParticle[i] = maxParticle;
//                for (int c = 0; c < 2; c++) {
//                    rgb[c][i] *= 2;
//                    if (rgb[c][i] > 1.0f) rgb[c][i] = 1.0f;
//                }
//                //rgb[0][i] = 1.0f; rgb[1][i] = 1.0f; rgb[2][i] = 1.0f;
//                SoundEngine->play2D("second_short.mp3", GL_FALSE);
//            }
//            else noexplosion2[i] = true;
//        }
//
//        if (numParticle[i] == 0) {
//            if (explosion[i]) {  // exposion finished
//
//                if (explosion2[i] || noexplosion2[i]) { // second explosion finished or no need to second explosion
//                    particles.erase(particles.begin() + i * maxParticle, particles.begin() + (i + 1) * maxParticle); // debug: 从begin开始erase的
//                    for (int k = i + 1; k < numFirework; k++) {
//                        numParticle[k - 1] = numParticle[k];
//                        posFirework[k - 1] = posFirework[k];
//                        posFire[k - 1] = posFire[k];
//                        explosion[k - 1] = explosion[k];
//                        explosion2[k - 1] = explosion2[k];
//                        noexplosion2[k - 1] = noexplosion2[k];
//                        rgb[0][k - 1] = rgb[0][k]; rgb[1][k - 1] = rgb[1][k]; rgb[2][k - 1] = rgb[2][k];
//                    }
//                    numFirework--;
//                    if (numFirework >= 0 && numFirework < maxFirework) {
//                        numParticle[numFirework] = 0;
//                        posFirework[numFirework] = 0;
//                        posFire[numFirework] = 0;
//                        explosion[numFirework] = false;
//                        explosion2[numFirework] = false;
//                        noexplosion2[numFirework] = false;
//                        rgb[0][numFirework] = -1.0f; rgb[1][numFirework] = -1.0f; rgb[2][numFirework] = -1.0f;
//                    }
//                    else cout << "error in after explosion finished" << endl;
//                }
//
//                else { // second explosion
//                    cout << "error in explosion2" << endl;
//                }
//
//            }
//
//            else { // not exposion yet
//                explosion[i] = true;
//                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) {
//                    Particle& particle = particles[j];
//                    float prob = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//                    if (particle.posY > posFirework[i] && prob > 0.97f)
//                        posFirework[i] = particle.posY; // the highest as the explosion pos at 50 prob
//                }
//                for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) {
//                    Particle& particle = particles[j];
//                    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;
//                    float rho = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f;
//                    float speedX = rho * cos(angle);
//                    float speedY = rho * sin(angle);
//                    float speedZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0004f - 0.0002f; speedZ = speedZ * (1 - rho * rho);
//                    particle.speedX = speedX; particle.speedY = speedY; particle.speedZ = speedZ;
//                    particle.alpha = 0.4f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.6f; particle.posY = posFirework[i];
//
//                }
//                numParticle[i] = maxParticle;
//                SoundEngine->play2D("explosion.mp3", GL_FALSE);
//            }
//        }
//
//    }
//}
//
//// Function to generate firework particles
//void generateParticles(float posX, float posY) {
//    for (int j = 0; j < maxParticle; j++) {
//        //float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
//        //float speedY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f + 0.0002f;
//        float speedY = log(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) / 1.2 * 0.001 + 0.002f;
//        //cout << speedY << '\n';
//        if (speedY < 0.0015)
//            speedY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0015f;
//        float speedX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.00005f - 0.000025f;
//        float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//        particles.push_back(Particle(posX, posY, 0.0f, speedX, speedY, 0.0f, alpha));
//    }
//    numParticle[numFirework] = maxParticle;
//    explosion[numFirework] = false;
//    explosion2[numFirework] = false;
//    noexplosion2[numFirework] = false;
//    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//    float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//    float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//
//    if (double(r) + double(g) + double(b) < 0.33) {
//        r = 2 * r + 0.3; g = 2 * g + 0.3; b = 2 * b + 0.3; // avoid too dark to see
//    }
//
//    rgb[0][numFirework] = r; rgb[1][numFirework] = g; rgb[2][numFirework] = b;
//    posFire[numFirework] = posX;
//    numFirework++;
//
//    SoundEngine->play2D("fire.wav", GL_FALSE);
//
//}
//
//// Function to update the firework position and size
//void update() {
//    //cout << "in update" << endl;
//
//    ed = clock();
//
//    float prop = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//
//    if (numFirework < maxFirework) {
//
//        if (numFirework == 0 || (float((ed - srt) / CLOCKS_PER_SEC) > maxInterval && prop > 0.3)) {
//            // Randomly generate a new firework position
//            fireworkPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f - 2.0f;
//            fireworkPosY = -0.5f;
//            //fireworkSize = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f + 5.0f;
//
//            // Generate firework particles
//            generateParticles(fireworkPosX, fireworkPosY);
//
//            srt = clock();
//        }
//        else if (float((ed - srt) / CLOCKS_PER_SEC) > maxInterval)
//            srt = clock();
//    }
//
//
//    updateParticles();
//
//
//    glutPostRedisplay();
//}
//
//// Function to draw a brown wooden bench
//void drawBench() {
//    // Draw the seat
//    glColor3f(0.612f, 0.42f, 0.188f);
//    glPushMatrix();
//    glTranslatef(0.0f, -0.2f, 0.0f);
//    glScalef(0.6f, 0.05f, 0.4f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//
//    // Draw the legs
//    glColor3f(0.435f, 0.29f, 0.184f);
//    glPushMatrix();
//    glTranslatef(-0.2f, -0.3f, -0.1f);
//    glScalef(0.05f, 0.2f, 0.05f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslatef(0.2f, -0.3f, -0.1f);
//    glScalef(0.05f, 0.2f, 0.05f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslatef(-0.2f, -0.3f, 0.1f);
//    glScalef(0.05f, 0.2f, 0.05f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslatef(0.2f, -0.3f, 0.1f);
//    glScalef(0.05f, 0.2f, 0.05f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//}
//
//// 函数power_of_two用于判断一个整数是不是2的整数次幂
//int power_of_two(int n)
//{
//    if (n <= 0)
//        return 0;
//    return (n & (n - 1)) == 0;
//}
//
//GLuint load_texture(const char* file_name)
//{
//    GLint width, height, total_bytes;
//    GLubyte* pixels = 0;
//    GLuint last_texture_ID = 0, texture_ID = 0;
//    FILE* pFile;
//    errno_t err;
//
//    // 打开文件，如果失败，返回
//    if ((err = fopen_s(&pFile, file_name, "rb")) != 0)
//    {
//        printf("无法打开此文件\n");            //如果打不开，就输出打不开
//        exit(0);                               //终止程序
//    }
//
//    // 读取文件中图象的宽度和高度
//    fseek(pFile, 0x0012, SEEK_SET);
//    fread(&width, 4, 1, pFile);
//    fread(&height, 4, 1, pFile);
//    fseek(pFile, BMP_Header_Length, SEEK_SET);
//
//    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
//    {
//        GLint line_bytes = width * 3;
//        while (line_bytes % 4 != 0)
//            ++line_bytes;
//        total_bytes = line_bytes * height;
//    }
//
//    // 根据总像素字节数分配内存
//    pixels = (GLubyte*)malloc(total_bytes);
//    if (pixels == 0)
//    {
//        fclose(pFile);
//        return 0;
//    }
//
//    // 读取像素数据
//    if (fread(pixels, total_bytes, 1, pFile) <= 0)
//    {
//        free(pixels);
//        fclose(pFile);
//        return 0;
//    }
//
//    // 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放
//    // 若图像宽高超过了OpenGL规定的最大值，也缩放
//    {
//        GLint max;
//        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
//        if (!power_of_two(width)
//            || !power_of_two(height)
//            || width > max
//            || height > max)
//        {
//            const GLint new_width = 256;
//            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
//            GLint new_line_bytes, new_total_bytes;
//            GLubyte* new_pixels = 0;
//
//            // 计算每行需要的字节数和总字节数
//            new_line_bytes = new_width * 3;
//            while (new_line_bytes % 4 != 0)
//                ++new_line_bytes;
//            new_total_bytes = new_line_bytes * new_height;
//
//            // 分配内存
//            new_pixels = (GLubyte*)malloc(new_total_bytes);
//            if (new_pixels == 0)
//            {
//                free(pixels);
//                fclose(pFile);
//                return 0;
//            }
//
//            // 进行像素缩放
//            gluScaleImage(GL_RGB,
//                width, height, GL_UNSIGNED_BYTE, pixels,
//                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);
//
//            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
//            free(pixels);
//            pixels = new_pixels;
//            width = new_width;
//            height = new_height;
//        }
//    }
//
//    // 分配一个新的纹理编号
//    glGenTextures(1, &texture_ID);
//    if (texture_ID == 0)
//    {
//        free(pixels);
//        fclose(pFile);
//        return 0;
//    }
//
//    // 绑定新的纹理，载入纹理并设置纹理参数
//    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
//    GLint lastTextureID = last_texture_ID;
//    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
//    glBindTexture(GL_TEXTURE_2D, texture_ID);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
//        GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
//    glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定
//    free(pixels);
//    return texture_ID;
//}
//
//
//// Function to display the scene
//void display() {
//    //cout << "in display" << endl;
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    // Enable depth testing
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND); // 打开混合
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 基于源象素alpha通道值的半透明混合函数
//
//    // Set up projection matrix
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
//
//    // Set up modelview matrix
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
//
//    // Adjust the overall ambient light
//    glEnable(GL_COLOR_MATERIAL);
//    glEnable(GL_LIGHTING);
//    GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
//
//    glEnable(GL_TEXTURE_2D);    // 启用纹理
//    //"D://code//VS code//ConsoleApplication1-v4.0//grass.bmp"
//
//    // Draw the sky
//    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
//
//    // Draw the park ground
//    //glColor4f(0.0f, 0.7f, 0.0f, 1.0f);
//    ///*glBegin(GL_QUADS);
//    //glVertex3f(-2.0f, -0.5f, 0.0f);
//    //glVertex3f(2.0f, -0.5f, 0.0f);
//    //glVertex3f(2.0f, -0.5f, 4.0f);
//    //glVertex3f(-2.0f, -0.5f, 4.0f);
//    //glEnd();*/
//    //glPushMatrix();
//    //glTranslatef(-0.0f, -1.0f, 2.0f);
//    //glScalef(4.0f, 1.0f, 4.0f);
//    //glutSolidCube(1.0);
//    //glPopMatrix();
//
//    glEnable(GL_TEXTURE_2D);    //草地纹理
//    glBindTexture(GL_TEXTURE_2D, texGround);
//    glBegin(GL_QUADS);
//    glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 0.0f);
//    glTexCoord2f(0.0f, 5.0f); glVertex3f(2.0f, -0.5f, 0.0f);
//    glTexCoord2f(5.0f, 5.0f); glVertex3f(2.0f, -0.5f, 4.0f);
//    glTexCoord2f(5.0f, 0.0f); glVertex3f(-2.0f, -0.5f, 4.0f);
//    glEnd();
//    glDisable(GL_TEXTURE_2D);
//
//    // Draw the street lamp
//    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
//    glPushMatrix();
//    glTranslatef(-0.95f, 0.6f, 1.0f);
//    glRotatef(90, 1, 0, 0);
//    glutSolidCone(0.1, 0.1, 10, 10);
//    glPopMatrix();
//
//    glPushMatrix();
//    glTranslatef(-0.95f, -0.0f, 1.0f);
//    glScalef(0.02f, 1.0f, 0.02f);
//    glutSolidCube(1.0);
//    glPopMatrix();
//
//    // Set up spotlight properties
//    GLfloat spotDirection[] = { 0.0f, -1.0f, 0.0f };
//    GLfloat spotExponent = 15.0f;
//    GLfloat spotCutoff = 45.0f;
//    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
//    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);
//    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotCutoff);
//    // Enable and set up a secondary light source for the lamp
//    glEnable(GL_LIGHT1);
//    GLfloat lampPosition[] = { -0.95f, 0.9f, 1.0f, 1.0f };
//    GLfloat lampAmbient[] = { 1.0f, 0.95f, 0.5f, 1.0f };
//    GLfloat lampDiffuse[] = { 1.0f, 0.95f, 0.5f, 1.0f };
//    GLfloat lampSpecular[] = { 1.0f, 0.0f, 0.0f, 1.0f };
//    glLightfv(GL_LIGHT1, GL_POSITION, lampPosition);
//    glLightfv(GL_LIGHT1, GL_AMBIENT, lampAmbient);
//    glLightfv(GL_LIGHT1, GL_DIFFUSE, lampDiffuse);
//    glLightfv(GL_LIGHT1, GL_SPECULAR, lampSpecular);
//
//
//    //// Set up spotlight properties
//    //GLfloat lightDirection[] = { 1.0f, 0.0f, 0.0f, -1.0f };
//    //GLfloat lightExponent = 10.0f;
//    //GLfloat lightCutoff = 180.0f;
//    //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
//    //glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, lightExponent);
//    //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, lightCutoff);
//    //// Enable and set up a secondary light source for the lamp
//    //glEnable(GL_LIGHT0);
//    //GLfloat lightPosition[] = { -1.0f, 0.0f, -0.5f, 1.0f };
//    //GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//    //GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 0.0f };
//    //GLfloat lightSpecular[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//    //glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
//    //glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
//    //glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
//    //glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
//
//    // Draw the firework    
//
//
//    // Modify the material properties for the fireworks
//    //GLfloat sun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的环境光颜色，为0
//    //GLfloat sun_mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的漫反射光颜色，为0
//    //GLfloat sun_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的镜面反射光颜色，为0
//    //GLfloat sun_mat_emission[] = { 1.0f, 1.0f, 1.0f, 1.0f };   //定义材质的辐射广颜色，为偏红色
//    //GLfloat sun_mat_shininess = 0.0f;
//    //glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
//    //glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
//    //glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
//    //glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
//    //glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
//
//    drawFirework();
//
//
//    GLfloat earth_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  //定义材质的环境光颜色，骗蓝色
//    GLfloat earth_mat_diffuse[] = { 0.0f, 0.0f, 0.5f, 1.0f };  //定义材质的漫反射光颜色，偏蓝色
//    GLfloat earth_mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的镜面反射光颜色，红色
//    GLfloat earth_mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };   //定义材质的辐射光颜色，为0
//    GLfloat earth_mat_shininess = 0.0f;
//    glMaterialfv(GL_FRONT, GL_AMBIENT, earth_mat_ambient);
//    glMaterialfv(GL_FRONT, GL_DIFFUSE, earth_mat_diffuse);
//    glMaterialfv(GL_FRONT, GL_SPECULAR, earth_mat_specular);
//    glMaterialfv(GL_FRONT, GL_EMISSION, earth_mat_emission);
//    glMaterialf(GL_FRONT, GL_SHININESS, earth_mat_shininess);
//
//    // Draw the bench
//    glPushMatrix();
//    glTranslatef(-0.5f, -0.1f, 1.0f);
//    drawBench();
//    glPopMatrix();
//
//
//
//    glutSwapBuffers();
//}
//
//
//
//int main(int argc, char** argv) {
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//    glutInitWindowSize(width, height);
//    glutCreateWindow("Firework Show in the Park");
//    // glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
//
//    texGround = load_texture("grass.bmp");
//
//    //initLight();
//
//    glutDisplayFunc(display);
//    glutIdleFunc(update);
//    glutMainLoop();
//
//    return 0;
//}


// version 2.5, init_light


//void initLight(void)
//{
//    //// Light properties
//    GLfloat lightPosition[] = { -0.95f, 0.5f, 1.0f, 1.0f };
//    GLfloat lightAmbient[] = { 0.0f, 0.2f, 0.2f, 0.5f };
//    GLfloat lightDiffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
//    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//    GLfloat lightDirection[] = { 1.0f, -1.0f, 0.0f, 1.0f };
//    GLfloat lightShininess[] = { 50.0f };
//    GLfloat lightCutoff[] = { 20.0f };
//
//    //// Enable lighting
//    //glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//
//    // Set light properties
//    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
//    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDirection);
//    //glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, lightCutoff);
//    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
//    //glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
//    //glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
//
//    //// Set material properties
//    //GLfloat matAmbient[] = { 0.6f, 0.6f, 0.6f, 1.0f };
//    //GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
//    //GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//    //glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
//    //glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
//    //glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
//    //glMaterialfv(GL_FRONT, GL_SHININESS, lightShininess);
//
//    glFrontFace(GL_CCW);
//    glEnable(GL_CULL_FACE);
//    // 启用光照计算
//    glEnable(GL_LIGHTING);
//    // 指定环境光强度（RGBA）
//    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//    // 设置光照模型，将ambientLight所指定的RGBA强度值应用到环境光
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
//    // 启用颜色追踪
//    glEnable(GL_COLOR_MATERIAL);
//    // 设置多边形正面的环境光和散射光材料属性，追踪glColor
//    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//
//}


// version 2, 5 firework and generate only < 5

//#include <stdlib.h>
//#include <GL/glut.h>
//#include <cmath>
//#include <vector>
//#include <iostream>
//
//// Window size
//int width = 800;
//int height = 600;
//
//// Firework properties
//float fireworkPosX = 0.0f;
//float fireworkPosY = -0.8f;
//float fireworkSize = 10.0f;
//
//const int maxFirework = 5; // max number of existing firework (only number of firework < maxFirework, new firework can be generated)
//const int maxParticle = 100; // the numbber of particle in a firework at the beginning
//const int maxInterval = 100;
//int numFirework = 0; // number of existing firework
//bool exposion[maxFirework];
//int numParticle[maxFirework], posFirework[maxFirework]; // number of visible points of each firework, position of exposion
//
//
//
//// Particle structure
//struct Particle {
//    float posX;
//    float posY;
//    float speedX;
//    float speedY;
//    float alpha;
//
//    Particle(float x, float y, float speed)
//        : posX(x), posY(y), speedY(speed), alpha(1.0f) {}
//};
//
//std::vector<Particle> particles;  // Vector to store particles
//
//// Function to draw a firework explosion
//void drawFirework(float posX, float posY, float size) {
//    glBegin(GL_POINTS);
//    for (int i = 0; i < particles.size(); i++) {
//        Particle& particle = particles[i];
//        glColor4f(1.0f, 1.0f, 1.0f, particle.alpha);
//        glVertex2f(particle.posX, particle.posY);
//    }
//    glEnd();
//}
//
//// Function to update the firework particles
//void updateParticles() {
//    for (int i = 0; i < particles.size() / maxParticle; i++) { // i-th firework
//
//        for (int j = i * maxParticle; j < i * maxParticle + maxParticle; j++) { //j-th particle
//
//
//            Particle& particle = particles[j];
//            particle.posY += particle.speedY;
//            particle.alpha -= 0.0005f;
//
//            if (particle.alpha <= 0.0f) {
//                numParticle[i]--;
//                /*particles.erase(particles.begin() + j);
//                j--;*/
//            }
//        }
//
//        if (numParticle[i] == 0) {
//            for (int j = i; j < i + maxParticle; j++)
//                particles.erase(particles.begin() + j);
//            for (int j = i + 1; j < numFirework; j++)
//                numParticle[j - 1] = numParticle[j];
//            numFirework--;
//        }
//
//    }
//}
//
//// Function to generate firework particles
//void generateParticles(float posX, float posY) {
//    for (int i = 0; i < 100; i++) {
//        //float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
//        float speed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.0002f + 0.0002f;
//        particles.push_back(Particle(posX, posY, speed));
//    }
//    numParticle[numFirework] = 100;
//    exposion[numFirework] = false;
//    numFirework++;
//
//}
//
//// Function to draw a brown wooden bench
//void drawBench() {
//    glColor4f(0.6f, 0.3f, 0.1f, 1.0f);
//    glBegin(GL_QUADS);
//    glVertex2f(-0.3f, -0.6f);
//    glVertex2f(0.3f, -0.6f);
//    glVertex2f(0.3f, -0.7f);
//    glVertex2f(-0.3f, -0.7f);
//    glEnd();
//}
//
//// Function to display the scene
//void display() {
//    //std::cout << "in display" << std::endl;
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    // Draw the sky
//    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
//
//    // Draw the park ground
//    glColor4f(0.0f, 0.5f, 0.0f, 1.0f);
//    glBegin(GL_QUADS);
//    glVertex2f(-1.0f, -0.5f);
//    glVertex2f(1.0f, -0.5f);
//    glVertex2f(1.0f, -1.0f);
//    glVertex2f(-1.0f, -1.0f);
//    glEnd();
//
//    // Draw the street lamp
//    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
//    glBegin(GL_QUADS);
//    glVertex2f(-0.6f, 0.0f);
//    glVertex2f(-0.6f, -0.6f);
//    glVertex2f(-0.55f, -0.6f);
//    glVertex2f(-0.55f, 0.0f);
//    glEnd();
//
//    // Draw the bench
//    glPushMatrix();
//    glTranslatef(-0.5f, -0.0f, 0.0f);
//    drawBench();
//    glPopMatrix();
//
//    // Draw the firework    
//    //glColor3f(1.0f, 1.0f, 1.0f);
//    glEnable(GL_BLEND); // 打开混合
//    //glDisable(GL_DEPTH_TEST); // 关闭深度测试
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 基于源象素alpha通道值的半透明混合函数
//    drawFirework(fireworkPosX, fireworkPosY, fireworkSize);
//
//    glutSwapBuffers();
//}
//
//// Function to update the firework position and size
//void update() {
//    //std::cout << "in update" << std::endl;
//
//    if (numFirework < maxFirework) {
//        // Randomly generate a new firework position
//        fireworkPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
//        fireworkPosY = -0.8f;
//        fireworkSize = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f + 5.0f;
//
//        // Generate firework particles
//        generateParticles(fireworkPosX, fireworkPosY);
//    }
//
//
//    updateParticles();
//
//
//    glutPostRedisplay();
//}
//
//
//int main(int argc, char** argv) {
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
//    glutInitWindowSize(width, height);
//    glutCreateWindow("Firework Show in the Park");
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
//    glutDisplayFunc(display);
//    glutIdleFunc(update);
//    glutMainLoop();
//
//    return 0;
//}

// version 1, before changing wo idlefunc
/*
#include <stdlib.h>
#include <GL/glut.h>
#include <cmath>
#include <vector>
#include <iostream>

// Window size
int width = 800;
int height = 600;

// Firework properties
float fireworkPosX = 0.0f;
float fireworkPosY = -0.8f;
float fireworkSize = 10.0f;

// Particle structure
struct Particle {
    float posX;
    float posY;
    float speedY;
    float alpha;

    Particle(float x, float y, float speed)
        : posX(x), posY(y), speedY(speed), alpha(1.0f) {}
};

std::vector<Particle> particles;  // Vector to store particles

// Function to draw a firework explosion
void drawFirework(float posX, float posY, float size) {
    glBegin(GL_POINTS);
    for (int i = 0; i < particles.size(); i++) {
        Particle& particle = particles[i];
        glColor4f(1.0f, 1.0f, 1.0f, particle.alpha);
        glVertex2f(particle.posX, particle.posY);
    }
    glEnd();
}

// Function to update the firework particles
void updateParticles() {
    for (int i = 0; i < particles.size(); i++) {
        Particle& particle = particles[i];
        particle.posY += particle.speedY;
        particle.alpha -= 0.01f;

        if (particle.alpha <= 0.0f) {
            particles.erase(particles.begin() + i);
            i--;
        }
    }
}

// Function to generate firework particles
void generateParticles(float posX, float posY) {
    for (int i = 0; i < 50; i++) {
        //float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
        float speed = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.02f + 0.02f;
        particles.push_back(Particle(posX, posY, speed));
    }
}

// Function to draw a brown wooden bench
void drawBench() {
    glColor3f(0.6f, 0.3f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(-0.3f, -0.6f);
    glVertex2f(0.3f, -0.6f);
    glVertex2f(0.3f, -0.7f);
    glVertex2f(-0.3f, -0.7f);
    glEnd();
}

// Function to display the scene
void display() {
    std::cout << "in display" << std::endl;
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the sky
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    // Draw the park ground
    glColor3f(0.0f, 0.5f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.5f);
    glVertex2f(1.0f, -0.5f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    // Draw the street lamp
    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(-0.6f, 0.0f);
    glVertex2f(-0.6f, -0.6f);
    glVertex2f(-0.55f, -0.6f);
    glVertex2f(-0.55f, 0.0f);
    glEnd();

    // Draw the bench
    glPushMatrix();
    glTranslatef(-0.5f, -0.7f, 0.0f);
    drawBench();
    glPopMatrix();

    // Draw the firework
    glColor3f(1.0f, 1.0f, 1.0f);
    drawFirework(fireworkPosX, fireworkPosY, fireworkSize);

    glutSwapBuffers();
}

// Function to update the firework position and size
void update(int value) {
    std::cout << "in update" << std::endl;
    // Randomly generate a new firework position
    fireworkPosX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f - 1.0f;
    fireworkPosY = -0.8f;
    fireworkSize = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f + 5.0f;

    // Generate firework particles
    generateParticles(fireworkPosX, fireworkPosY);

    glutPostRedisplay();
    glutTimerFunc(2000, update, 0);  // Update every 2 seconds
}

// Timer function to update particles
void timer(int value) {
    std::cout << "in timer" << std::endl;
    updateParticles();
    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);  // Update particles every 30 milliseconds
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Firework Show in the Park");
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glutDisplayFunc(display);
    glutTimerFunc(2000, update, 0);
    glutTimerFunc(10, timer, 0);
    glutMainLoop();

    return 0;
}

*/
