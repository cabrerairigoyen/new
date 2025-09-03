#ifndef PI_STREAM_CONTROLLER_H
#define PI_STREAM_CONTROLLER_H

#include <escher.h>
#include <ion.h>
#include <ion/timing.h>
#include <ion/console.h>
#include <escher/expression_view.h>
#include <poincare.h>

namespace PiStream {

class PiStreamController : public StackViewController {
public:
  PiStreamController(Responder * parentResponder);
  const char * title() override { return "Pi Stream Display"; }
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;

private:
  void pollUART();
  void appendToBuffer(char c);
  void processBuffer();
  void appendText(const char * text);

  char m_buffer[1024];
  TextView m_textView;
  ScrollableView m_scrollableTextView;
  ExpressionView m_expressionView;
  KDCoordinate m_lastPollTime;
};

}

#endif
