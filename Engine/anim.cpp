#include "entities.h"

Keyframe::Keyframe():time(0.0f),value(0){}

Influence::Influence():
	influence_controlpointindex(0),
	influence_ncontrolpointindex(0),
	influence_weight(0.0f)
	{}

Cluster::Cluster():
	cluster_bone(0),
	cluster_influences(NULL),
	cluster_ninfluences(0)
	{}

KeyCurve::KeyCurve():
	keycurve_channel(INVALID_CHANNEL),
	keycurve_start(-1),
	keycurve_end(-1)
{}


CurveGroup::CurveGroup():
	curvegroup_start(-1),
	curvegroup_end(-1)
{}

Animation::Animation():
	animation_animselected(0),
	animation_nprocessed(0),
	animation_time(0),
	animation_direction(0),
	animation_start(-1),
	animation_end(-1),
	animation_scl(1,1,1)
{}