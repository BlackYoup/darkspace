#ifndef GADGET_REDUCE_RANGE_H
#define GADGET_REDUCE_RANGE_H

#include "Constants.h"
#include "NounGadget.h"
#include "NounShip.h"
#include "GameDll.h"

class DLL GadgetReduceRange : public NounGadget
{
public:
	DECLARE_WIDGET_CLASS();
	DECLARE_PROPERTY_LIST();

	GadgetReduceRange();

	Type type() const;
	dword hotkey() const;

	virtual CharString useTip(Noun * pTarget, bool shift) const;
	bool usable(Noun * pTarget, bool shift) const;
	void use(dword when, Noun * pTarget, bool shift);
	bool useActive() const;
	int useEnergy(dword nTick, int energy);

	// Accessors
	virtual int				energyCost() const = 0;			// energy cost to use 
	virtual float			range() const = 0;				// area affected
	void			release();

	bool active() const;
	float reducePercent() const;

protected:
	NounShip::wRef m_Target = NULL;
};

inline bool GadgetReduceRange::active() const {
	return (flags() & FLAG_ACTIVE) != 0;
}

inline float GadgetReduceRange::reducePercent() const {
	return 10.0f;
}

#endif