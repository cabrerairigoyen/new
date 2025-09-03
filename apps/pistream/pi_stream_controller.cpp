#include "pi_stream_controller.h"
#include <assert.h>
#include <string.h>

// Para n0110 necesitamos usar UART GPIO directamente
// Incluimos las definiciones necesarias para acceder a USART6
#include <ion/device/shared/drivers/config/console.h>

namespace PiStream {

PiStreamController::PiStreamController(Responder * parentResponder) :
  StackViewController(parentResponder, &m_scrollableTextView, Pane::None),
  m_scrollableTextView(&m_textView),
  m_textView(KDFont::SmallFont),
  m_lastPollTime(0)
{
  m_buffer[0] = 0;
}

void PiStreamController::viewWillAppear() {
  m_buffer[0] = 0;
  m_textView.setText(m_buffer);
  m_lastPollTime = Ion::Timing::millis();

  // Clear stack and push initial views
  popAll();
  push(&m_scrollableTextView);

  // Mostrar mensaje inicial
  m_textView.setText("Pi Stream v2 UART GPIO\n\nConecte Raspberry Pi y presione OK");
}

bool PiStreamController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Event::OK) {
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

  // Para n0110: Usar UART GPIO directamente via USART6
  // Acceso directo a registros para comunicación no-bloqueante
  using namespace Ion::Device::Console;

  // Check if receive data register is not empty (RXNE flag)
  bool dataAvailable = Config::Port.SR()->getRXNE();

  if (dataAvailable) {
    // Read available characters (non-blocking)
    char receivedLine[256] = {0};
    int charCount = 0;

    // Read all available characters without blocking
    while (Config::Port.SR()->getRXNE() && charCount < 255) {
      char c = (char)Config::Port.RDR()->get();

      if (c == '\n' || c == '\r') {
        if (charCount > 0) {
          receivedLine[charCount] = '\0';
          // Procesar línea completa
          processReceivedData(receivedLine);
          charCount = 0;
        }
      } else {
        receivedLine[charCount++] = c;
      }
    }

    // Process any remaining data in buffer
    if (charCount > 0) {
      receivedLine[charCount] = '\0';
      processReceivedData(receivedLine);
    }
  }
}

void PiStreamController::processReceivedData(const char * data) {
  // Copiar datos al buffer principal
  int len = strlen(m_buffer);
  if (len < sizeof(m_buffer) - strlen(data) - 1) {
    strcpy(m_buffer + len, data);
    strcpy(m_buffer + len + strlen(data), "\n");
  } else {
    // Buffer lleno, hacer scroll
    memmove(m_buffer, m_buffer + strlen(data) + 1, sizeof(m_buffer) - strlen(data) - 1);
    strcpy(m_buffer + sizeof(m_buffer) - strlen(data) - 2, data);
    strcpy(m_buffer + sizeof(m_buffer) - 2, "\n");
  }

  // Look for LaTeX delimiters (e.g., $$latex$$ or \(latex\))
  char * start = strstr(m_buffer, "$$");
  if (!start) start = strstr(m_buffer, "\\(");
  if (start) {
    char * end = strstr(start + 2, "$$");
    if (!end) end = strstr(start + 2, "\\)");
    if (end) {
      *end = 0;  // Null-terminate LaTeX string
      Poincare::Expression expr = Poincare::Expression::Parse(start + 2, nullptr);
      if (!expr.isUninitialized()) {
        // Render math using proper ExpressionView
        Poincare::Layout layout = expr.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, Poincare::Preferences::ComplexFormat::Real);
        m_expressionView.setLayout(layout);
        push(&m_expressionView);
        return; // Don't append as text
      }
    }
  }

  // No LaTeX found, just update text display
  m_textView.setText(m_buffer);
  m_scrollableTextView.scrollToBottom();
}

void PiStreamController::appendToBuffer(char c) {
  int len = strlen(m_buffer);
  if (len < sizeof(m_buffer) - 1) {
    m_buffer[len] = c;
    m_buffer[len + 1] = 0;
  } else {
    memmove(m_buffer, m_buffer + 1, sizeof(m_buffer) - 1);
    m_buffer[sizeof(m_buffer) - 1] = 0;
  }
}

void PiStreamController::processBuffer() {
  // Look for LaTeX delimiters (e.g., $$latex$$ or \(latex\))
  char * start = strstr(m_buffer, "$$");
  if (!start) start = strstr(m_buffer, "\\(");
  if (start) {
    char * end = strstr(start + 2, "$$");
    if (!end) end = strstr(start + 2, "\\)");
    if (end) {
      *end = 0;  // Null-terminate LaTeX string
      Poincare::Expression expr = Poincare::Expression::Parse(start + 2, nullptr);
      if (!expr.isUninitialized()) {
        // Render math using proper ExpressionView
        Poincare::Layout layout = expr.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, Poincare::Preferences::ComplexFormat::Real);
        m_expressionView.setLayout(layout);
        push(&m_expressionView);
        return; // Don't append as text
      } else {
        // Invalid: Treat as text
        appendText(start);
      }
      // Shift buffer past processed part
      memmove(m_buffer, end + 2, strlen(end + 2) + 1);
      return;
    }
  }
  // No LaTeX: Append as text if newline found
  char * nl = strchr(m_buffer, '\n');
  if (nl) {
    *nl = 0;
    appendText(m_buffer);
    memmove(m_buffer, nl + 1, strlen(nl + 1) + 1);
  }
}

void PiStreamController::appendText(const char * text) {
  // Note: TextView text handling might need different approach
  // This is a simplified version - actual implementation may need buffer management
  m_textView.setText(text);
  m_scrollableTextView.scrollToBottom();
}

}
