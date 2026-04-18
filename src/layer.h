#pragma once

class ILayer {
public:
	/* This method will be called AFTER attaching */
	virtual void onAttach() = 0;
	/* This one will be called BEFORE the layer will be detached */
	virtual void onDetach() = 0;
	virtual void onUpdate() = 0;
};