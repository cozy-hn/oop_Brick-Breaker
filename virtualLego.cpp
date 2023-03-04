////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: ��â�� Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>

IDirect3DDevice9* Device = NULL;

// window size
const int Width  = 1024;
const int Height = 768;

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982

// initialize the position (coordinate) of each ball
const float spherePos[6][2] = { {-2.0f,3.0f} , {-0.7f,2.5f} , {2.0f,2.5f} , {0.7f,3.0f},{.0f,-5.0f+0.1f+M_RADIUS},{.0f,-5.0f+0.11f+3*M_RADIUS}}; 
// initialize the color of each ball
const D3DXCOLOR sphereColor[6] = {d3d::YELLOW, d3d::YELLOW, d3d::YELLOW, d3d::YELLOW,d3d::BLUE,d3d::MAGENTA};


// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private :
	float center_x, center_y, center_z;
    float m_radius;
	float m_velocity_x;
	float m_velocity_z;

public:
    CSphere(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_radius = M_RADIUS;
		m_velocity_x = 0;
		m_velocity_z = 0;
        m_pSphereMesh = NULL;
    }
    ~CSphere(void) {}

public:
    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
            return false;
        return true;
    }
	
    void destroy(void)
    {
        if (m_pSphereMesh != NULL) {
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pSphereMesh->DrawSubset(0);
    }
	
	bool hasIntersected(CSphere& ball)
	{
		// Insert your code here.
		float Distanceball = (float)(pow((center_x - ball.center_x), 2) + pow((center_z - ball.center_z), 2)); //두 공 사이의 거리
		if (Distanceball < (M_RADIUS*2)*(M_RADIUS*2)) return true; //충돌시 중심 거리의 반이 반지름이므로 
		return false;
	}

	void hitBy(CSphere& ball) 
	{   
		if (hasIntersected(ball)){//속도 고정하면서 방향 변하게 설정

		    float dx =(ball.getCenter().x - center_x);
		    float dz = (ball.getCenter().z - center_z);
		    float Adjustment_constant;

		    float size_V = sqrt(ball.getVelocity_X() * ball.getVelocity_X() + ball.getVelocity_Z() * ball.getVelocity_Z());
		    float size_D = sqrt(dx * dx + dz * dz);
            Adjustment_constant = size_V / size_D;

		    float new_V_x = Adjustment_constant * dx;
		    float new_V_z = Adjustment_constant * dz;

		    ball.setPower(new_V_x, new_V_z);
        }    
    }

	void ballUpdate(float timeDiff) 
	{
		const float TIME_SCALE = 3.3;
		D3DXVECTOR3 cord = this->getCenter();
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		if(vx > 0.01 || vz > 0.01)
		{
			float tX = cord.x + TIME_SCALE*timeDiff*m_velocity_x;
			float tZ = cord.z + TIME_SCALE*timeDiff*m_velocity_z;

			
			this->setCenter(tX, cord.y, tZ);
		}
	}

	double getVelocity_X() { return this->m_velocity_x;	}
	double getVelocity_Z() { return this->m_velocity_z; }

	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x=x;	center_y=y;	center_z=z;
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}
	
	float getRadius(void)  const { return (float)(M_RADIUS);  }
    const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; }
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
    D3DXVECTOR3 getCenter(void) const
    {
        D3DXVECTOR3 org(center_x, center_y, center_z);
        return org;
    }

    void setRadius(float radius){
		this->m_radius=radius;
	}
	
private:
    D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pSphereMesh;
	
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:
	
    float					m_x;
	float					m_z;
	float                   m_width;
    float                   m_depth;
	float					m_height;
	
public:
    CWall(void)
    {
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }
    ~CWall(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
    {
        if (NULL == pDevice)
            return false;
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
        m_mtrl.Specular = color;
        m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power    = 5.0f;
		
        m_width = iwidth;
        m_depth = idepth;
		
        if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
            return false;
        return true;
    }
    void destroy(void)
    {
        if (m_pBoundMesh != NULL) {
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }
    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return;
        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);
		m_pBoundMesh->DrawSubset(0);
    }

	
	bool hasIntersected(CSphere& ball) 
	{
		// Insert your code here.
        //긴 바
        if (ball.getCenter().x >= (3 - M_RADIUS))  return true;
        if (ball.getCenter().x <= (-3 + M_RADIUS)) return true;
        //짧은 바
        if (ball.getCenter().z <= (-5 + M_RADIUS))	 return true;
        if (ball.getCenter().z >= (5 - M_RADIUS))	 return true;
		return false;
	}

	void hitBy(CSphere& ball) 
	{
		// Insert your code here.
        //벽에 충돌 시 속도변화 고려

    if(hasIntersected(ball)){
        //긴 바
        if (ball.getCenter()[0] >= (3 - M_RADIUS)) {
            ball.setCenter((3 - M_RADIUS),ball.getCenter().y,ball.getCenter().z);
            ball.setPower(-ball.getVelocity_X(),ball.getVelocity_Z());
        }
        if (ball.getCenter()[0] <= (-3 + M_RADIUS)){
            ball.setCenter((-3 + M_RADIUS),ball.getCenter().y,ball.getCenter().z);
            ball.setPower(-ball.getVelocity_X(),ball.getVelocity_Z());
        }
        //짧은 바
        if (ball.getCenter()[2] <= (-5 + M_RADIUS))	{
            ball.setCenter(ball.getCenter().x,ball.getCenter().y,(-5 + M_RADIUS));
            ball.setPower(ball.getVelocity_X(),-ball.getVelocity_Z());
        };
        if (ball.getCenter()[2] >= (5 - M_RADIUS)){
            ball.setCenter(ball.getCenter().x,ball.getCenter().y,(5 - M_RADIUS));
            ball.setPower(ball.getVelocity_X(),-ball.getVelocity_Z());
        }
        }
    }    
	
	
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}


	
    float getHeight(void) const { return M_HEIGHT; }
	
	
	
private :
    void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; }
	
	D3DXMATRIX              m_mLocal;
    D3DMATERIAL9            m_mtrl;
    ID3DXMesh*              m_pBoundMesh;
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
    CLight(void)
    {
        static DWORD i = 0;
        m_index = i++;
        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));
        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }
    ~CLight(void) {}
public:
    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
    {
        if (NULL == pDevice)
            return false;
        if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
            return false;
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;
		
        m_lit.Type          = lit.Type;
        m_lit.Diffuse       = lit.Diffuse;
        m_lit.Specular      = lit.Specular;
        m_lit.Ambient       = lit.Ambient;
        m_lit.Position      = lit.Position;
        m_lit.Direction     = lit.Direction;
        m_lit.Range         = lit.Range;
        m_lit.Falloff       = lit.Falloff;
        m_lit.Attenuation0  = lit.Attenuation0;
        m_lit.Attenuation1  = lit.Attenuation1;
        m_lit.Attenuation2  = lit.Attenuation2;
        m_lit.Theta         = lit.Theta;
        m_lit.Phi           = lit.Phi;
        return true;
    }
    void destroy(void)
    {
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }
    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
    {
        if (NULL == pDevice)
            return false;
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);
        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);
        return true;
    }

    void draw(IDirect3DDevice9* pDevice)
    {
        if (NULL == pDevice)
            return;
        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);
        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
    DWORD               m_index;
    D3DXMATRIX          m_mLocal;
    D3DLIGHT9           m_lit;
    ID3DXMesh*          m_pMesh;
    d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane;
CWall	g_legowall[4];
CSphere	g_sphere[6];
// CSphere	g_target_blueball;
// CSphere g_target_magentaball;
CSphere g_target_whiteball;

CLight	g_light;

double g_camera_pos[3] = {0.0, 5.0, -8.0};


// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void){}

// initialization
bool Setup()
{
	int i;
	
    D3DXMatrixIdentity(&g_mWorld);
    D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mProj);
		
	// create plane and set the position
    //create(pDevice, ix, iz, iwidth, iheight, idepth, color)
    if (false == g_legoPlane.create(Device, -1, -1, 6, 0.03f, 10, d3d::GREEN)) return false;
    g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);
	
	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0].create(Device, -1, -1, 6, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 5.0f);
	if (false == g_legowall[1].create(Device, -1, -1, 0.12f, 0.3f, 10.0f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(-3.0f, 0.12f, 0.0f);
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 10.0f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(3.0f, 0.12f, 0.0f);
	if (false == g_legowall[3].create(Device, -1, -1, 6, 0.0f, 0.12f, d3d::GREEN)) return false;
	g_legowall[3].setPosition(0.0f, 0.12f, -5.0f);

	// create 4 balls and set the position
	for (i=0;i<4;i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false;
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS , spherePos[i][1]);
		g_sphere[i].setPower(0,0);
	}
	
	// create blue ball for set direction
    if (false == g_sphere[4].create(Device, sphereColor[4])) return false;
	g_sphere[4].setCenter(spherePos[4][0], (float)M_RADIUS , spherePos[4][1]);
    g_sphere[4].setPower(0,0);
	// create magenta ball for set direction
    if (false == g_sphere[5].create(Device, sphereColor[5])) return false;
	g_sphere[5].setCenter(spherePos[5][0], (float)M_RADIUS ,spherePos[5][1]);
    g_sphere[5].setPower(0,0);

    if (false == g_target_whiteball.create(Device, d3d::WHITE)) return false;
	g_target_whiteball.setCenter(.0f, (float)M_RADIUS , 5.2f);
    g_target_whiteball.setPower(0,0);


	// light setting 
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
    lit.Type         = D3DLIGHT_POINT;
    lit.Diffuse      = d3d::WHITE; 
	lit.Specular     = d3d::WHITE * 0.9f;
    lit.Ambient      = d3d::WHITE * 0.9f;
    lit.Position     = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
    lit.Range        = 100.0f;
    lit.Attenuation0 = 0.0f;
    lit.Attenuation1 = 0.9f;
    lit.Attenuation2 = 0.0f;
    if (false == g_light.create(Device, lit))
        return false;
	
	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 10.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 4.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4, (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set render states.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
    g_legoPlane.destroy();
	for(int i = 0 ; i < 4; i++) {
		g_legowall[i].destroy();
	}
    destroyAllLegoBlock();
    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i=0;
	int j = 0;
    static int cnt = 0;

	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		
		// update the position of each ball. during update, check whether each ball hit by walls.

		for( i = 0; i < 4; i++) {
			g_sphere[i].ballUpdate(timeDelta);
            g_legowall[i].hitBy(g_sphere[5]);
            if (g_sphere[5].getCenter().z <= (-4.99 + M_RADIUS)) {//MAGENTABALL이 바닥에 닿으면 게임 종료
                g_sphere[5].setCenter(-15.0f, M_RADIUS, -15.0f);
                g_sphere[5].setPower(.0f, .0f);
            }
        }
		

        g_sphere[4].ballUpdate(timeDelta);
        g_sphere[5].ballUpdate(timeDelta);
        g_target_whiteball.ballUpdate(timeDelta);

		for(i = 0 ;i < 4; i++){
            if(g_sphere[i].hasIntersected(g_sphere[5])){
                g_sphere[i].hitBy(g_sphere[5]);
                g_sphere[i].setCenter(-15.0f, M_RADIUS,-15.0f);
                g_sphere[i].setPower(.0f, .0f);
                cnt++;
                if (cnt == 4) {
                    g_sphere[5].setCenter(-15.0f, M_RADIUS, -15.0f);
                    g_sphere[5].setPower(.0f, .0f);
                }

            }
		}

        //파란공 고정
        D3DXVECTOR3 coord3d = g_sphere[4].getCenter();
        g_sphere[4].hitBy(g_sphere[5]);
        g_sphere[4].setCenter(coord3d.x, coord3d.y, coord3d.z);
        g_sphere[4].setPower(0.0, 0.0);

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);

		for (i=0;i<4;i++) 	{
			g_legowall[i].draw(Device, g_mWorld);
            g_sphere[i].draw(Device, g_mWorld);
		}

        g_sphere[4].draw(Device, g_mWorld);
		g_sphere[5].draw(Device, g_mWorld);
        g_target_whiteball.draw(Device, g_mWorld);
        g_light.draw(Device);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );

        // g_target_magentaball.ballUpdate(timeDelta);
	}

	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
    static int old_x = 0;
    static int old_y = 0;
    static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;
    int x,y,z;
    static int i = 0;
	switch( msg ) {
	case WM_DESTROY:
        {
			::PostQuitMessage(0);
			break;
        }
	case WM_KEYDOWN:
        {
            switch (wParam) {
            case VK_ESCAPE:
				::DestroyWindow(hwnd);
                break;
            case VK_RETURN:
                if (NULL != Device) {
                    wire = !wire;
                    Device->SetRenderState(D3DRS_FILLMODE,
                        (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
                }
                break;

            case VK_SPACE:
			{
                if (i == 0) {//처음 한 번만 스페이스 누르기 기능 
				    D3DXVECTOR3 targetpos = g_target_whiteball.getCenter();
				    D3DXVECTOR3	whitepos = g_sphere[5].getCenter();
				    double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					    pow(targetpos.z - whitepos.z, 2)));		// �⺻ 1 ��и�
				    if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { theta = -theta; }	//4 ��и�
				    if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { theta = PI - theta; } //2 ��и�
				    if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0){ theta = PI + theta; } // 3 ��и�
				    double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				    g_sphere[5].setPower(distance * cos(theta)*(0.3), distance * sin(theta)*(0.3));
                    g_target_whiteball.setCenter(-15.0f, 0, -15.0f);
                    i = 1;
                }
				
                break;
            }

			

            }
    }
        
       

	case WM_MOUSEMOVE:
        {
            int new_x = LOWORD(lParam);
            int new_y = HIWORD(lParam);
			float dx;
			float dy;
			
            if (LOWORD(wParam) & MK_RBUTTON) {
				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;
		
				D3DXVECTOR3 coord3d=g_target_whiteball.getCenter();
				g_target_whiteball.setCenter(coord3d.x+dx*(-0.01f),coord3d.y,coord3d.z);
				
				
                old_x = new_x;
                old_y = new_y;

            }
            else {
                isReset = true;
				
				if (LOWORD(wParam) & MK_LBUTTON) {
					dx = (old_x - new_x);// * 0.01f;
					dy = (old_y - new_y);// * 0.01f;
		
					D3DXVECTOR3 coord3d=g_sphere[4].getCenter();
					g_sphere[4].setCenter(coord3d.x+dx*(-0.01f),coord3d.y,coord3d.z);
				}
				old_x = new_x;
				old_y = new_y;
				
                move = WORLD_MOVE;
            }
            break;
        }
	}

	
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}