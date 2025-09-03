apps += PiStream::App
app_headers += apps/pistream/app.h

SFLAGS += -DHAS_PISTREAM

app_pistream_src = $(addprefix apps/pistream/,\
  app.cpp \
  pi_stream_controller.cpp \
)

apps_src += $(app_pistream_src)

app_images += apps/pistream/pi_stream_icon.png

i18n_files += $(call i18n_without_universal_for,pistream/base)

$(eval $(call depends_on_image,apps/pistream/app.cpp,apps/pistream/pi_stream_icon.png))
