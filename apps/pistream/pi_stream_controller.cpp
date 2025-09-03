#include "pi_stream_controller.h"
#include <assert.h>
#include <string.h>

// Implementaciones de funciones de string para toolchain embebido
// (ya que no están disponibles en la librería estándar)

// #include <cstring>  // No disponible en toolchain embebido

// Implementación simple de strcpy
char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

// Implementación simple de memcpy
void *memcpy(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--) *d++ = *s++;
    return dest;
}

// Implementación simple de memmove
void *memmove(void *dest, const void *src, size_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

// Implementación simple de strstr
char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    for (const char *h = haystack; *h; ++h) {
        const char *h_iter = h;
        const char *n_iter = needle;

        while (*h_iter && *n_iter && *h_iter == *n_iter) {
            ++h_iter;
            ++n_iter;
        }

        if (!*n_iter) return (char *)h;
    }

    return nullptr;
}

// Implementación simple de strchr
char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        ++s;
    }
    return (c == '\0') ? (char *)s : nullptr;
}

// strlen ya debería estar disponible, pero por si acaso
size_t my_strlen(const char *s) {
    size_t len = 0;
    while (*s++) ++len;
    return len;
}

// Para n0110 necesitamos usar UART GPIO directamente
// Configuración UART independiente sin dependencias del sistema de registros roto

// Definiciones básicas para STM32F730 (usado en NumWorks n0110)
#define USART6_BASE 0x40011400
#define GPIOC_BASE  0x40020800

// Estructura básica de registros USART (simplificada)
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
} USART_TypeDef;

// Estructura básica de registros GPIO (simplificada)
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

// Punteros a periféricos (sin reinterpret_cast en define para constexpr)
#define USART6_BASE_ADDR (USART6_BASE)
#define GPIOC_BASE_ADDR  (GPIOC_BASE)

// Configuración específica para n0110 UART GPIO
namespace Ion {
namespace Device {
namespace Console {
namespace Config {

// Punteros a registros (definidos en runtime, no constexpr)
static USART_TypeDef* Port = reinterpret_cast<USART_TypeDef*>(USART6_BASE);
static GPIO_TypeDef* GPIOPort = reinterpret_cast<GPIO_TypeDef*>(GPIOC_BASE);

constexpr static uint32_t RxPin = 7;  // PC7
constexpr static uint32_t TxPin = 6;  // PC6
constexpr static uint32_t AlternateFunction = 8;  // AF8

// Baudrate: 115200 con fAPB2 = 96 MHz
// USARTDIV = f/BaudRate = 96000000/115200 = 833.333
constexpr static int USARTDIVValue = 833;

}
}
}
}

namespace PiStream {

PiStreamController::PiStreamController(Responder * parentResponder) :
  ViewController(parentResponder),
  m_textView(KDFont::SmallFont),
  m_scrollableTextView(parentResponder, &m_textView, nullptr),
  m_lastPollTime(0)
{
  m_buffer[0] = 0;
  // StackViewController setup moved to viewWillAppear
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

  // Para n0110: Usar UART GPIO directamente via configuración independiente
  // Check if receive data register is not empty (RXNE flag en bit 5 del ISR)
  bool dataAvailable = (Ion::Device::Console::Config::Port->ISR & (1 << 5)) != 0;

  if (dataAvailable) {
    // Read available characters (non-blocking)
    char receivedLine[256] = {0};
    int charCount = 0;

    // Read all available characters without blocking
    while ((Ion::Device::Console::Config::Port->ISR & (1 << 5)) && charCount < 255) {
      char c = (char)(Ion::Device::Console::Config::Port->RDR & 0xFF);

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
  size_t len = my_strlen(m_buffer);
  size_t data_len = my_strlen(data);
  if (len < sizeof(m_buffer) - data_len - 1) {
    strcpy(m_buffer + len, data);
    strcpy(m_buffer + len + data_len, "\n");
  } else {
    // Buffer lleno, hacer scroll
    memmove(m_buffer, m_buffer + data_len + 1, sizeof(m_buffer) - data_len - 1);
    strcpy(m_buffer + sizeof(m_buffer) - data_len - 2, data);
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
        Poincare::Layout layout = expr.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7); // 7 significant digits
        m_expressionView.setLayout(layout);
        // For now, just append as text since we can't push ExpressionView directly
        appendText(start + 2);
        return; // Don't append as text again
      }
    }
  }

  // No LaTeX found, just update text display
  m_textView.setText(m_buffer);
  // ScrollableView doesn't have scrollToBottom() method
}

void PiStreamController::appendToBuffer(char c) {
  size_t len = my_strlen(m_buffer);
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
        Poincare::Layout layout = expr.createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7); // 7 significant digits
        m_expressionView.setLayout(layout);
        // For now, just append as text since we can't push ExpressionView directly
        appendText(start + 2);
        return; // Don't append as text again
      } else {
        // Invalid: Treat as text
        appendText(start);
      }
      // Shift buffer past processed part
      memmove(m_buffer, end + 2, my_strlen(end + 2) + 1);
      return;
    }
  }
  // No LaTeX: Append as text if newline found
  char * nl = strchr(m_buffer, '\n');
  if (nl) {
    *nl = 0;
    appendText(m_buffer);
    memmove(m_buffer, nl + 1, my_strlen(nl + 1) + 1);
  }
}

void PiStreamController::appendText(const char * text) {
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
