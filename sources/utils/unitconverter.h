#pragma once

#include <QtGlobal>

/// OLVS-version edition
/**
	@brief The UnitConverter class
	Centralizes all conversions between pixels (QET's internal drawing unit)
	and real-world physical units (millimeters, inches).

	QET operates at a fixed 96 DPI, so:
	  1 px = 25.4 / 96 mm  (~0.2646 mm)
	  1 px = 1 / 96 in     (exact)
*/
class UnitConverter
{
	public:
		enum class Unit { Millimeter, Inch };

		static constexpr qreal DPI = 96.0;

		static qreal pxToMm(qreal px);
		static qreal mmToPx(qreal mm);

		static qreal pxToIn(qreal px);
		static qreal inToPx(qreal in);

		static qreal pxTo(qreal px, Unit unit);
		static qreal toPx(qreal value, Unit unit);

		/// Converts a physical value already in one unit to another.
		static qreal convert(qreal value, Unit from, Unit to);

	private:
		UnitConverter() = delete;
};
