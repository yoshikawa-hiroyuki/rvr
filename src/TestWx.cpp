#ifdef __GNUG__
#pragma implementation
#pragma interface
#endif
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <stdio.h>
#include <vector>

#include "rvrVolumeRenderer.h"
#include "vfrScreen.h"
#include "vfrCamera3D.h"
#include "vfrScene.h"
#include "vfrGroup.h"
#include "vfrCube.h"
#include "vfrDefaultActions.h"

#include "rvrUtils.h"

#include "TestWx.h"


using namespace std;
using namespace CES;

vfrScreen* screen;
vfrCamera3D* camera;
vfrScene* scene;
vfrGroup* root;
rvrVolumeRenderer* s_render;

Bool READ_DATA(const char* fname)
{
  rvrSimpleLoader loader(fname);
  if ( ! loader.m_data ) return FALSE;
  int dims[3];
  dims[0] = rvrSimpleLoader::WrapPow2(loader.m_size[0]);
  dims[1] = rvrSimpleLoader::WrapPow2(loader.m_size[1]);
  dims[2] = rvrSimpleLoader::WrapPow2(loader.m_size[2]);
  if ( ! loader.Resample(dims[0], dims[1], dims[2]) ) return FALSE;

  s_render = new rvrVolumeRenderer(0.01f, 2000.01f, 256, true);
  s_render->SetVolume(loader.m_data, dims[0], dims[1], dims[2]);
  s_render->SetBbox(loader.m_bbox[0], loader.m_bbox[1]);

  return TRUE;
}

Bool GEN_DATA()
{
  const int     sx = 128;
  const int     sy = 128;
  const int     sz = 256;

  vector<unsigned char> data;
  data.resize( sx * sy * sz  );

  const double  kr = 0.16;
  const double  kd = 6.0;
  double        dx, dy, dz;
  for ( int z = 0; z < sz; z++ ) {
    dz = kr * ( z - ( sz / 2 ));
    for ( int y = 0; y < sy; y++ ) {
      dy = kr * ( y - ( sy / 2 ));
      for ( int x = 0; x < sx; x++ ) {
        dx = kr * ( x - ( sx / 2 ));
        double  r = sqrt( dx * dx + dy * dy + dz * dz );
        double  cos_theta = dz / r;
        double  phi = kd * ( r * r ) * exp( -r / 2 )
          * ( 3 * cos_theta * cos_theta - 1 );
        double  c = phi * phi;
        if ( c > 255.0 ) {
          c = 255.0;
        }
        data[(x + sx * ( y + sy * z ))] = static_cast<unsigned char>( c );
      }
    }
  }

  s_render = new rvrVolumeRenderer(0.01f, 2000.01f, 256, true);
  s_render->SetVolume(&data[0], sx, sy, sz );

  return TRUE;
}

class VRender : public vfrNode {
public:
  VRender() : p_render(NULL) {
    alcMaterial();
    _material->setSpecular(1.f, 1.f, 1.f, 1.f);
    setAlpha(TRUE);
    notice();
  }
  virtual ~VRender() {}

  virtual void renderSolid() {
    if ( ! p_render ) return;

    glDisable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );

    p_render->Draw();
  }
  virtual void renderWire() {
    if ( ! p_render ) return;
    //glColor3f(1.f, 1.f, 1.f);
    //p_render->DrawBbox();
  }

  rvrVolumeRenderer* p_render;
};

VRender* render;


bool TestApp::OnInit()
{
  /* Create the main frame window */
  TestFrame *frame = new TestFrame(NULL, wxT("TestWx"),
                                   wxPoint(50, 50), wxSize(400, 300));
  
  /* Make a menubar */
  wxMenu *fileMenu = new wxMenu;

  fileMenu->Append(wxID_EXIT, wxT("E&xit"));
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, wxT("&File"));
  frame->SetMenuBar(menuBar);

  vfrDrawAreaWx* da = vfrDrawAreaWx::GetInstance(frame);
  frame->SetCanvas(da->getCanvas());

  /* Show the frame */
  frame->Show(TRUE);
#ifdef MACOSX
  da->getCanvas()->MakeCurrent();
#endif

  /* scene graph */
  vfrScreen* screen = new vfrScreen();
  da->addScreen(screen);
  vfrCamera3D *camera = new vfrCamera3D();
  camera->setProjection(VFR::PR_ORTHOGONAL);
  
  screen->setCamera(camera);
  scene = new vfrScene();
  camera->setScene(scene);
  root = new vfrGroup("ROOT");
  scene->addChild(root);

  vfrLight *l0 = scene->getLight(0);
  l0->setLightType(LT_POINT);
  l0->trans(0.0, 0.0, 100.0);

  render = new VRender();
#if 1
  //if ( ! READ_DATA("orm_volume.sph") ) return FALSE;
  if ( ! READ_DATA("hydrogen.dat") ) return FALSE;
#else
  if ( ! GEN_DATA() ) return FALSE;
#endif

  rvrLUT rlut;
  //for (int i = 0; i < 256; i++ ) rlut.m_rgba[i][3] = 255;
  s_render->SetLUT(rlut);
  //rlut.m_rgba[0][3] = 0;
  for (int i = 0; i < 256; i++ ) rlut.m_rgba[i][3] = 255;
  for (int i = 0; i < 1; i++ ) rlut.m_rgba[i][3] = 0;
  s_render->SetLUT(rlut, true);

  render->p_render = s_render;
  root->addChild(render);

  //vfrDefaultActions::SetDefaultAction(*da);
  vfrDispatch &dispatcher = vfrDispatch::instance(*da);
  vfrEvKeyIn::instance(dispatcher).regist(&vfrKeyInAction::instance());
  vfrEvSDrag::instance(dispatcher).regist(&vfrTransNodeAction::instance());
  vfrEvCDrag::instance(dispatcher).regist(&vfrRotNodeAction::instance());
  vfrEvSCDrag::instance(dispatcher).regist(&vfrScaleNodeAction::instance());
  vfrDefaultActions::SetSelectedNode(render);

  return TRUE;
}

IMPLEMENT_APP(TestApp)

BEGIN_EVENT_TABLE(TestFrame, wxFrame)
  EVT_MENU(wxID_EXIT, TestFrame::OnExit)
END_EVENT_TABLE()



TestFrame::TestFrame(wxFrame *frame, const wxString& title,
                     const wxPoint& pos, const wxSize& size, long style)
  : wxFrame(frame, -1, title, pos, size, style)
{
  m_canvas = NULL;

  SetIcon(vfrDrawAreaWx::GetVFRIcon());
}

/* Intercept menu commands */
void TestFrame::OnExit(wxCommandEvent& event)
{
  Destroy();
}
