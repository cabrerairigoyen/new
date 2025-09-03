#ifndef PI_STREAM_CONTROLLER_H
#define PI_STREAM_CONTROLLER_H

#include <escher.h>
#include <ion.h>
#include <ion/timing.h>
#include <ion/console.h>
#include <escher/expression_view.h>
#include <escher/buffer_text_view.h>
#include <escher/alternate_empty_view_delegate.h>
#include <poincare/expression.h>
#include <poincare/layout.h>
#include <poincare/preferences.h>

namespace PiStream {

class PiStreamController : public StackViewController, public AlternateEmptyViewDelegate {
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

  // AlternateEmptyViewDelegate methods
  bool isEmpty() const override { return true; }
  View * emptyView() override;
  Responder * defaultController() override { return this; }

  char m_buffer[1024];
  BufferTextView m_textView;
  ScrollableView m_scrollableTextView;
  ExpressionView m_expressionView;
  KDCoordinate m_lastPollTime;
};

}

#endif
