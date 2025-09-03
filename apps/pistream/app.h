#ifndef PISTREAM_APP_H
#define PISTREAM_APP_H

#include <escher.h>
#include "pi_stream_controller.h"

namespace PiStream {

class App : public ::App {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    I18n::Message name() override;
    I18n::Message upperName() override;
    const Image * icon() override;
  };

  class Snapshot : public ::App::Snapshot {
  public:
    App * unpack(Container * container) override;
    Descriptor * descriptor() override;
  };

private:
  App(Snapshot * snapshot);
  PiStreamController m_piStreamController;
  AlternateEmptyViewController m_alternateEmptyViewController;
  StackViewController m_stackViewController;
};

}

#endif
