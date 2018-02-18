#include "Debug/Assert.h"
#include "Math/Helpers.h"
#include "NounShip.h"
#include "NounBeacon.h"
#include "GadgetReduceRange.h"
#include "GadgetDrive.h"
#include "GadgetReactor.h"
#include "GadgetFighterBay.h"
#include "GameContext.h"
#include "VerbGadgetOff.h"

IMPLEMENT_ABSTRACT_FACTORY(GadgetReduceRange, NounGadget);
BEGIN_ABSTRACT_PROPERTY_LIST(GadgetReduceRange, NounGadget);
	ADD_TRANSMIT_PROPERTY(m_Target);
	ADD_REPLICATED_PROPERTY(m_Flags, TICKS_PER_SECOND);
END_PROPERTY_LIST();

GadgetReduceRange::GadgetReduceRange()
{
}

NounGadget::Type GadgetReduceRange::type() const {
	return SPECIAL_OFFENSIVE;
}

dword GadgetReduceRange::hotkey() const {
	return 'W';
}

CharString GadgetReduceRange::useTip(Noun * pTarget, bool shift) const {
	CharString tip;
	tip += CharString().format("\nRange:<X;100>%.0fgu", range() * calculateModifier(MT_WEAPON_RANGE));
	tip += CharString().format("\nEnergy Cost:<X;100>%.1f", energyCost() * calculateModifier( MT_EWAR_ENERGY, true));
	if (active())
		tip += CharString().format("\nTarget:<X;100>%s", m_Target->displayName(false));
	return tip;
}

bool GadgetReduceRange::usable(Noun * pTarget, bool shift) const {
	if (active())
		return true;	// always allow it to be turned off...
	if (!NounGadget::usable(pTarget, shift))
		return false;

	NounShip *pTargetShip = WidgetCast<NounShip>(pTarget);

	if (!pTargetShip)
		return false;

	if (((NounShip *)pTarget)->isDestroyed())
		return false;

	if (!isEnemy(pTargetShip))
		return false;

	if (pTargetShip->testFlags( NounShip::FLAG_CLOAKED|NounShip::FLAG_IN_SAFE_ZONE ) )
		return false;

	if (active() && m_Target != pTargetShip)
		return false;

	float distance = (worldPosition() - pTarget->worldPosition()).magnitude();
	if (distance > range() * calculateModifier(MT_WEAPON_RANGE))
		return false;        // target out of rang

	return true;
}

bool GadgetReduceRange::useActive() const
{
	return active();
}

void GadgetReduceRange::use(dword when, Noun * pTarget, bool shift)
{
	if (active())
	{
		NounGadget::use(when, pTarget, shift);
		m_Target->addModifier(MT_WEAPON_RANGE, reducePercent());
		if (isServer())
		{
			setFlags(FLAG_ACTIVE, false);
			message( CharString().format( "<color;ffffff>Tactical: %s Inactive on %s", name(), m_Target->name() ), true );  
		}
	}
	else
	{
		NounShip *pShipTarget = WidgetCast<NounShip>(pTarget);
		if (!shift && pShipTarget != NULL)
		{
			m_Target = pShipTarget;
			m_Target->subtractModifier(MT_WEAPON_RANGE, reducePercent());
			if (isServer())
			{
				setFlags(FLAG_ACTIVE, true);
				message(CharString().format("<color;ffffff>Tactical: %s Active on %s", name(), m_Target->name()), true);
			}
		}
	}
}
int GadgetReduceRange::useEnergy( dword nTick, int energy )
{
	if ( active() )
	{
		bool bTurnOff = true;

		if ( m_Target.valid() && !m_Target->isDestroyed() )
		{
			int nCost = energyCost() * calculateModifier( MT_EWAR_ENERGY, true);
			float fDistance = (worldPosition() - m_Target->worldPosition()).magnitude();
			float fRange = range() * calculateModifier( MT_WEAPON_RANGE );

			if ( fDistance <= fRange && energy >= nCost )
			{
				bTurnOff = false;

				energy -= nCost;
			}
		}

		if ( bTurnOff && isLocal() )
	        Verb::Ref( new VerbGadgetOff( this ) );
	}

	return energy;
}

void GadgetReduceRange::release()
{
	NounGadget::release();
	if (active())
	{
		setFlags(FLAG_ACTIVE, false);
		m_Target->addModifier(MT_WEAPON_RANGE, reducePercent());
		m_Target = NULL;
	}
}