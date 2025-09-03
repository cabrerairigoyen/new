#include "app.h"
#include "pi_stream_icon.h"
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
  return ImageStore::PiStreamIcon;
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
  m_piStreamController(&m_stackViewController),
  m_alternateEmptyViewController(&m_stackViewController, &m_piStreamController, &m_piStreamController),
  m_stackViewController(&m_alternateEmptyViewController, &m_piStreamController)
{
  // Initialize the stack with the PiStream controller
  m_stackViewController.push(&m_piStreamController);
}

int App::numberOfTimers() {
  return 0;
}

I18n::Message App::message() {
  return I18n::Message::Default;
}

void * App::frameBuffer() {
  return nullptr;
}

}
