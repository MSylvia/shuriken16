#include "theme.h"

namespace Theme
{
	QColor background = QColor(42, 42, 42);
	QColor backgroundDark = QColor(16, 16, 16);
	QColor backgroundWindow = QColor(54, 54, 54);
	QColor backgroundHighlight = QColor(74, 74, 74);
	QColor content = QColor(224, 224, 224);
	QColor light = QColor(120, 120, 120);
	QColor disabled = QColor(144, 144, 144);
	QColor selection = QColor(96, 96, 96);
	QColor selectionLight = QColor(128, 128, 128);
	QColor green = QColor(162, 217, 175);
	QColor red = QColor(222, 143, 151);
	QColor blue = QColor(128, 198, 233);
	QColor cyan = QColor(142, 230, 237);
	QColor lightCyan = QColor(176, 221, 228);
	QColor orange = QColor(237, 189, 129);
	QColor yellow = QColor(237, 223, 179);
	QColor magenta = QColor(218, 196, 209);
}


QColor MixColor(const QColor& a, const QColor& b, uint8_t alpha)
{
	uint8_t red = (uint8_t)((((uint16_t)a.red() * (255 - alpha)) +
		((uint16_t)b.red() * alpha)) / 255);
	uint8_t green = (uint8_t)((((uint16_t)a.green() * (255 - alpha)) +
		((uint16_t)b.green() * alpha)) / 255);
	uint8_t blue = (uint8_t)((((uint16_t)a.blue() * (255 - alpha)) +
		((uint16_t)b.blue() * alpha)) / 255);
	return QColor(red, green, blue);
}
