#include "app.h"
// #include "pi_stream_icon.h"  // Archivo no generado
#include <apps/apps_container.h>
#include <apps/i18n.h>

namespace PiStream {

I18n::Message App::Descriptor::name() {
  return I18n::Message::PiStreamApp;
}

I18n::Message App::Descriptor::upperName() {
  return I18n::Message::PiStreamAppCapital;
}

const Image * App::Descriptor::icon() {
  return nullptr; // Temporal: sin icono para evitar crashes
}

App * App::Snapshot::unpack(Container * container) {
  return new (container->currentAppBuffer()) App(this);
}

App::Descriptor * App::Snapshot::descriptor() {
  static Descriptor descriptor;
  return &descriptor;
}

App::App(Snapshot * snapshot) :
  ::App(snapshot, &m_piStreamController),
  m_piStreamController(this)
{
  // Safe initialization with error checking
  if (!snapshot) {
    // Handle null snapshot gracefully
    m_piStreamController.viewWillAppear();
  }
}

}
