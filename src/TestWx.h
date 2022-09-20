#ifndef _WX_TESTWX_H_
#define _WX_TESTWX_H_

#include "vfrDrawAreaWx.h"

/* Define a new application type */
class TestApp: public wxApp
{
public:
    bool OnInit(void);
};


/* Define a new frame type */
class TestFrame: public wxFrame
{
public:
  TestFrame(wxFrame *frame, const wxString& title,
            const wxPoint& pos, const wxSize& size,
            long style = wxDEFAULT_FRAME_STYLE);
  
  void OnExit(wxCommandEvent& event);
  
  void SetCanvas(vfrWxCanvas* canvas) {m_canvas = canvas;}
  vfrWxCanvas* GetCanvas() {return m_canvas;}
  
private:
  vfrWxCanvas*    m_canvas;

  DECLARE_EVENT_TABLE()
};

#endif // _WX_TESTWX_H_
