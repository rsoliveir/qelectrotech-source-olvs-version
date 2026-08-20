#include "unitconverter.h"

/// OLVS-version edition
qreal UnitConverter::pxToMm(qreal px)
{
	return px * 25.4 / DPI;
}

qreal UnitConverter::mmToPx(qreal mm)
{
	return mm * DPI / 25.4;
}

qreal UnitConverter::pxToIn(qreal px)
{
	return px / DPI;
}

qreal UnitConverter::inToPx(qreal in)
{
	return in * DPI;
}

qreal UnitConverter::pxTo(qreal px, Unit unit)
{
	switch (unit) {
		case Unit::Inch:
			return pxToIn(px);
		case Unit::Millimeter:
		default:
			return pxToMm(px);
	}
}

qreal UnitConverter::toPx(qreal value, Unit unit)
{
	switch (unit) {
		case Unit::Inch:
			return inToPx(value);
		case Unit::Millimeter:
		default:
			return mmToPx(value);
	}
}

qreal UnitConverter::convert(qreal value, Unit from, Unit to)
{
	if (from == to) return value;
	// Round-trip through px keeps this correct regardless of how many
	// units get added later, at the cost of one extra multiplication.
	return pxTo(toPx(value, from), to);
}
