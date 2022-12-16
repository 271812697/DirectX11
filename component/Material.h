#pragma once
#include<d3d11.h>
#include<memory>
#include<../EffectHelper.h>
namespace component {


	class Material{
	private:
		std::shared_ptr<EffectHelper>effect;
	public:
		void Bind()const;
		void SetEffect(std::shared_ptr<EffectHelper> effect_ref);
		
	
	};
}