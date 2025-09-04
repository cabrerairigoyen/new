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

class PiStreamController : public ViewController, public AlternateEmptyViewDelegate {
public:
  PiStreamController(Responder * parentResponder);
  const char * title() override { return "Pi Stream Display"; }
  View * view() override;
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;

private:
  void pollUART();
  void processReceivedData(const char * data);
  void appendToBuffer(char c);
  void processBuffer();
  void appendText(const char * text);
  void safeBufferShift(const char * newStart);
  void emergencyReset();

  // AlternateEmptyViewDelegate methods
  bool isEmpty() const override { return true; }
  View * emptyView() override;
  Responder * defaultController() override { return this; }

  char m_buffer[1024];
  BufferTextView m_textView;
  ScrollableView m_scrollableTextView;
  ExpressionView m_expressionView;
  KDCoordinate m_lastPollTime;
  KDCoordinate m_readStartTime; // For timeout protection
  KDCoordinate m_lastProcessingTime; // For watchdog mechanism
  int m_processingCounter; // Prevent infinite loops
};

}

#endif
