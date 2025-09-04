#include "pi_stream_controller.h"
#include <assert.h>
#include <string.h>

namespace PiStream {

PiStreamController::PiStreamController(Responder * parentResponder) :
  ViewController(parentResponder),
  m_textView(KDFont::SmallFont),
  m_scrollableTextView(parentResponder, &m_textView, nullptr),
  m_lastPollTime(0),
  m_readStartTime(0),
  m_lastProcessingTime(0),
  m_processingCounter(0)
{
  m_buffer[0] = 0;
  // StackViewController setup moved to viewWillAppear
}

View * PiStreamController::view() {
  return &m_scrollableTextView;
}

void PiStreamController::viewWillAppear() {
  m_buffer[0] = 0;
  m_textView.setText(m_buffer);
  m_lastPollTime = Ion::Timing::millis();

  // Mostrar mensaje inicial
  m_textView.setText("Pi Stream v2 UART GPIO\n\nConecte Raspberry Pi y presione OK");

  ViewController::viewWillAppear();
}

bool PiStreamController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK) {
    // Activar/desactivar UART
    pollUART();
    return true;
  }
  if (event == Ion::Events::Back) {
    return false; // Let parent handle back button
  }
  pollUART();
  return true;
}

void PiStreamController::pollUART() {
  KDCoordinate currentTime = Ion::Timing::millis();
  if (currentTime - m_lastPollTime < 50) return;
  m_lastPollTime = currentTime;

  // Non-blocking UART read with timeout protection
  // Use Ion::Console::available() if available, otherwise implement timeout
  static KDCoordinate lastReadAttempt = 0;
  const KDCoordinate READ_TIMEOUT = 1000; // 1 second timeout

  // Prevent excessive read attempts that could cause system freeze
  if (currentTime - lastReadAttempt < 10) return; // Minimum 10ms between attempts
  lastReadAttempt = currentTime;

  // Check for timeout to prevent infinite blocking
  static bool readInProgress = false;
  if (readInProgress && (currentTime - m_readStartTime > READ_TIMEOUT)) {
    readInProgress = false;
    appendText("[UART Timeout - Connection may be lost]\n");
    return;
  }

  // Non-blocking UART read without exception handling (not supported)
  if (!readInProgress) {
    readInProgress = true;
    m_readStartTime = currentTime;

    // Try non-blocking read - if no data available, readChar() will return immediately
    char c = Ion::Console::readChar();
    readInProgress = false;

    // Only process if we got a valid character (not null/EOF)
    if (c != 0 && c != -1) {
      appendToBuffer(c);
      processBuffer();
    }
  }
}

void PiStreamController::processReceivedData(const char * data) {
  // Safe buffer management with bounds checking
  if (!data) return;

  size_t len = strlen(m_buffer);
  size_t data_len = strlen(data);

  // Prevent buffer overflow
  if (data_len >= sizeof(m_buffer)) {
    data_len = sizeof(m_buffer) - 1;
  }

  if (len + data_len + 1 < sizeof(m_buffer)) {
    strcpy(m_buffer + len, data);
    strcpy(m_buffer + len + data_len, "\n");
  } else {
    // Buffer full, safe scroll
    size_t shiftAmount = data_len + 1;
    if (len > shiftAmount) {
      memmove(m_buffer, m_buffer + shiftAmount, len - shiftAmount + 1);
      len -= shiftAmount;
    }
    if (len + data_len + 1 < sizeof(m_buffer)) {
      strcpy(m_buffer + len, data);
      strcpy(m_buffer + len + data_len, "\n");
    }
  }

  // Look for LaTeX delimiters with bounds checking
  char * start = strstr(m_buffer, "$$");
  if (!start) start = strstr(m_buffer, "\\(");

  if (start && start < m_buffer + len - 2) {
    char * end = nullptr;
    if (start[0] == '$' && start[1] == '$') {
      end = strstr(start + 2, "$$");
    } else if (start[0] == '\\' && start[1] == '(') {
      end = strstr(start + 2, "\\)");
    }

    if (end && end < m_buffer + len) {
      *end = '\0';

      const char * mathText = start + 2;
      size_t mathLen = end - (start + 2);

      if (mathLen > 0 && mathLen < 256) {
        Poincare::Expression expr = Poincare::Expression::Parse(mathText, nullptr);
        if (!expr.isUninitialized()) {
          Poincare::Layout layout = expr.createLayout(
            Poincare::Preferences::PrintFloatMode::Decimal,
            Poincare::Preferences::ComplexFormat::Real
          );
          m_expressionView.setLayout(layout);
          appendText(start + 2);
          return;
        }
      }
      appendText("[Math Error]");
      return;
    }
  }

  // No LaTeX found, just update text display
  m_textView.setText(m_buffer);
}

void PiStreamController::appendToBuffer(char c) {
  // Safe buffer management with bounds checking
  size_t len = strlen(m_buffer);
  if (len >= sizeof(m_buffer) - 1) {
    // Buffer full, shift left to make space (safe operation)
    size_t shiftAmount = 1;
    if (len > 0) {
      memmove(m_buffer, m_buffer + shiftAmount, len - shiftAmount + 1);
      len -= shiftAmount;
    }
  }

  // Add new character with bounds check
  if (len < sizeof(m_buffer) - 1) {
    m_buffer[len] = c;
    m_buffer[len + 1] = '\0';
  }
}

void PiStreamController::processBuffer() {
  // Safe LaTeX processing with bounds checking
  size_t bufferLen = strlen(m_buffer);
  if (bufferLen == 0) return;

  // Look for LaTeX delimiters with bounds checking
  char * start = strstr(m_buffer, "$$");
  if (!start) start = strstr(m_buffer, "\\(");

  if (start && start < m_buffer + bufferLen - 2) { // Ensure we have space for delimiters
    char * end = nullptr;
    if (start[0] == '$' && start[1] == '$') {
      end = strstr(start + 2, "$$");
    } else if (start[0] == '\\' && start[1] == '(') {
      end = strstr(start + 2, "\\)");
    }

    if (end && end < m_buffer + bufferLen) {
      // Safe null termination
      *end = '\0';

      // Safe expression parsing without exception handling
      const char * mathText = start + 2;
      size_t mathLen = end - (start + 2);

      // Check for reasonable math expression length
      if (mathLen > 0 && mathLen < 256) {
        Poincare::Expression expr = Poincare::Expression::Parse(mathText, nullptr);
        if (!expr.isUninitialized()) {
          // Safe layout creation
          Poincare::Layout layout = expr.createLayout(
            Poincare::Preferences::PrintFloatMode::Decimal,
            Poincare::Preferences::ComplexFormat::Real
          );
          m_expressionView.setLayout(layout);
          // For now, just append the math result as text since we can't push ExpressionView
          appendText("[Math Result]");
          // Safe buffer shift
          safeBufferShift(end + 2);
          return;
        }
      }
      // Invalid expression or too long: treat as regular text
      appendText(start);
      safeBufferShift(end + 2);
      return;
    }
  }

  // No LaTeX: Append as text if newline found
  char * nl = strchr(m_buffer, '\n');
  if (nl && nl < m_buffer + bufferLen) {
    *nl = '\0';
    appendText(m_buffer);
    // Safe buffer shift
    safeBufferShift(nl + 1);
  }
}

void PiStreamController::safeBufferShift(const char * newStart) {
  // Safe buffer shifting with bounds checking
  if (!newStart || newStart < m_buffer || newStart >= m_buffer + sizeof(m_buffer)) {
    return; // Invalid pointer
  }

  size_t remainingLength = strlen(newStart);
  if (remainingLength >= sizeof(m_buffer)) {
    remainingLength = sizeof(m_buffer) - 1; // Prevent overflow
  }

  // Safe memory move
  memmove(m_buffer, newStart, remainingLength);
  m_buffer[remainingLength] = '\0';
}

void PiStreamController::emergencyReset() {
  // Emergency reset function to recover from critical errors
  m_buffer[0] = '\0';
  m_lastPollTime = Ion::Timing::millis();
  m_readStartTime = 0;
  m_lastProcessingTime = Ion::Timing::millis();
  m_processingCounter = 0;

  // Show recovery message
  m_textView.setText("Pi Stream - Emergency Reset\nCheck Raspberry Pi connection");
}

void PiStreamController::appendText(const char * text) {
  // Safe text appending with null check
  if (!text) {
    text = "[NULL]";
  }

  // Note: TextView text handling might need different approach
  // This is a simplified version - actual implementation may need buffer management
  m_textView.setText(text);
  // ScrollableView doesn't have scrollToBottom() method
}

View * PiStreamController::emptyView() {
  m_textView.setText("Conecte Raspberry Pi y presione OK para iniciar Pi Stream");
  return &m_textView;
}

}

