#include "FresponzeMasterEmitter.h"

#ifdef STEAM_AUDIO_BUILD
IPLhandle context = nullptr;
long ContextCounter = 0;

IBaseEmitter*
GetSteamEmitter()
{
	return new CSteamAudioEmitter;
}

void
CSteamAudioEmitter::Reset()
{
	if (BinauralEffect) iplDestroyBinauralEffect(&BinauralEffect);
	if (BinauralRender) iplDestroyBinauralRenderer(&BinauralRender);
	if (OutputBuffer.deinterleavedBuffer) {
		for (size_t i = 0; i < outputFormat.Channels; i++) {
			if (OutputBuffer.deinterleavedBuffer[i]) FreeFastMemory(OutputBuffer.deinterleavedBuffer[i]);
		}

		FreeFastMemory(OutputBuffer.deinterleavedBuffer);
	}
	if (InputBuffer.deinterleavedBuffer) {
		for (size_t i = 0; i < outputFormat.Channels; i++) {
			if (InputBuffer.deinterleavedBuffer[i]) FreeFastMemory(InputBuffer.deinterleavedBuffer[i]);
		}

		FreeFastMemory(InputBuffer.deinterleavedBuffer);
	}
	if (ContextCounter <= 0) {
		iplDestroyContext(&context);
		context = nullptr;
	}
}

void
CSteamAudioEmitter::Create()
{
	/* Steam audio API settings apply */
	EmitterSettings.convolutionType = IPL_CONVOLUTIONTYPE_PHONON;
	EmitterSettings.frameSize = outputFormat.Frames;
	EmitterSettings.samplingRate = outputFormat.SampleRate;
	hrtfParams.type = IPL_HRTFDATABASETYPE_DEFAULT;
	iplCreateBinauralRenderer(context, EmitterSettings, hrtfParams, &BinauralRender);
	EmitterFormat.channelLayoutType = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
	EmitterFormat.channelLayout =
		outputFormat.Channels == 2 ? IPL_CHANNELLAYOUT_STEREO : outputFormat.Channels == 4 ? IPL_CHANNELLAYOUT_QUADRAPHONIC :
		outputFormat.Channels == 6 ? IPL_CHANNELLAYOUT_FIVEPOINTONE : outputFormat.Channels == 8 ? IPL_CHANNELLAYOUT_SEVENPOINTONE :
		outputFormat.Channels == 1 ? IPL_CHANNELLAYOUT_MONO : IPL_CHANNELLAYOUT_STEREO;
	EmitterFormat.channelOrder = IPL_CHANNELORDER_DEINTERLEAVED;

	iplCreateBinauralEffect(BinauralRender, EmitterFormat, EmitterFormat, &BinauralEffect);
	InputBuffer.format = EmitterFormat;
	OutputBuffer.format = EmitterFormat;
	InputBuffer.numSamples = outputFormat.Frames;
	OutputBuffer.numSamples = outputFormat.Frames;
	InputBuffer.deinterleavedBuffer = (IPLfloat32**)FastMemAlloc(outputFormat.Channels * sizeof(void*));
	OutputBuffer.deinterleavedBuffer = (IPLfloat32**)FastMemAlloc(outputFormat.Channels * sizeof(void*));
	for (size_t i = 0; i < outputFormat.Channels; i++) {
		InputBuffer.deinterleavedBuffer[i] = (IPLfloat32*)FastMemAlloc(outputFormat.Frames * sizeof(fr_f32));
		OutputBuffer.deinterleavedBuffer[i] = (IPLfloat32*)FastMemAlloc(outputFormat.Frames * sizeof(fr_f32));
	}
}

CSteamAudioEmitter::CSteamAudioEmitter()
{
	AddRef();
	if (!context) iplCreateContext(nullptr, nullptr, nullptr, &context);
	ContextCounter++;
}

CSteamAudioEmitter::~CSteamAudioEmitter()
{
	IMediaListener* pTemp = ((IMediaListener*)pParentListener);
	_RELEASE(pTemp);
	ContextCounter--;
	Reset(); 
	FreeStuff();
}

void 
CSteamAudioEmitter::FreeStuff()
{
	EffectNodeStruct* pNode = pFirstEffect;
	EffectNodeStruct* pThisNode = nullptr;
	while (pNode) {
		pThisNode = pNode->pNext;
		_RELEASE(pNode->pEffect);
		delete pNode;
		pNode = pThisNode;
	}
}

void 
CSteamAudioEmitter::AddEffect(IBaseEffect* pNewEffect)
{
	if (!pLastEffect) {
		pFirstEffect = new EffectNodeStruct;
		memset(pFirstEffect, 0, sizeof(EffectNodeStruct));
		pLastEffect = pFirstEffect;
		pNewEffect->Clone((void**)&pLastEffect->pEffect);
	}
	else {
		EffectNodeStruct* pTemp = new EffectNodeStruct;
		memset(pTemp, 0, sizeof(EffectNodeStruct));
		pNewEffect->Clone((void**)&pTemp->pEffect);
		pLastEffect->pNext = pTemp;
		pTemp->pPrev = pLastEffect;
		pLastEffect = pTemp;
	}
}

void 
CSteamAudioEmitter::DeleteEffect(IBaseEffect* pNewEffect)
{
	EffectNodeStruct* pNode = pFirstEffect;
	while (pNode) {
		if (pNode->pEffect == pNewEffect) {
			pNode->pPrev->pNext = pNode->pNext;
			pNode->pNext->pPrev = pNode->pPrev;
			_RELEASE(pNode->pEffect);
			delete pNode;
			return;
		}
	}
}

void
CSteamAudioEmitter::SetFormat(PcmFormat* pFormat)
{
	ListenerFormat = *pFormat;
	EffectNodeStruct* pNEffect = pFirstEffect;
	while (pNEffect) {
		pNEffect->pEffect->SetFormat(pFormat);
		pNEffect = pNEffect->pNext;
	}
}

void
CSteamAudioEmitter::GetFormat(PcmFormat* pFormat)
{
	*pFormat = ListenerFormat;
}

void	
CSteamAudioEmitter::SetListener(void* pListener) 
{
	IMediaListener* pMediaListener = (IMediaListener*)pListener;
	pMediaListener->Clone(&pParentListener);
	pMediaListener->GetFormat(outputFormat);
	Create();
}

void	
CSteamAudioEmitter::SetState(fr_i32 state)
{
	EmittersState = state;
}

void	
CSteamAudioEmitter::SetPosition(fr_i64 FPosition) 
{
	FilePosition = FPosition;
}

void*	
CSteamAudioEmitter::GetListener()
{
	return pParentListener;
}

fr_i32	
CSteamAudioEmitter::GetState()
{
	return EmittersState;
}

fr_i64	
CSteamAudioEmitter::GetPosition()
{
	return FilePosition;
}

bool	
CSteamAudioEmitter::GetEffectCategory(fr_i32& EffectCategory)
{
	return true;
}

bool	
CSteamAudioEmitter::GetEffectType(fr_i32& EffectType) 
{
	return true;
}

bool	
CSteamAudioEmitter::GetPluginName(fr_string64& DescriptionString)
{
	strcpy(DescriptionString, "Steam Audio Emitter");
	return true;
}

bool	
CSteamAudioEmitter::GetPluginVendor(fr_string64& DescriptionString) 
{
	strcpy(DescriptionString, "Valve Inc.");
	return true;
}

bool	
CSteamAudioEmitter::GetPluginDescription(fr_string256& DescriptionString) 
{
	strcpy(DescriptionString, "Steam Audio Emitter for HRTF audio");
	return true;
}

bool	
CSteamAudioEmitter::GetVariablesCount(fr_i32& CountOfVariables) 
{
	CountOfVariables = eCountOfParameters;
	return true;
}

bool	
CSteamAudioEmitter::GetVariableDescription(fr_i32 VariableIndex, fr_string128& DescriptionString) 
{
	if (VariableIndex >= ePluginParametersCount) return false;
	return true;
}

bool	
CSteamAudioEmitter::GetVariableKnob(fr_i32 VariableIndex, fr_i32& KnobType)
{
	if (VariableIndex >= ePluginParametersCount) return false;
	return true;
}

void	
CSteamAudioEmitter::SetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize) 
{
	if (!pData) return;
	if (Option >= ePluginParametersCount) return;
	if (DataSize != sizeof(fr_f32)) return;
	fr_f32 ValueToApply = *pData;

	switch (Option) {
	case eSVolumeParameter:		VolumeLevel = ValueToApply; break;
	case eXAxis:				vectorOfAngle.x = ValueToApply; break;
	case eYAxis:				vectorOfAngle.y = ValueToApply; break;
	case eZAxis:				vectorOfAngle.z = ValueToApply; break;
	default:
		break;
	}
}

void	
CSteamAudioEmitter::GetOption(fr_i32 Option, fr_f32* pData, fr_i32 DataSize)
{
	if (!pData) return;
	if (Option >= ePluginParametersCount) return;
	if (DataSize != sizeof(fr_f32)) return;
	fr_f32& ValueToApply = *pData;

	switch (Option) {
	case eSVolumeParameter:		ValueToApply = VolumeLevel; break;
	case eXAxis:				ValueToApply = vectorOfAngle.x; break;
	case eYAxis:				ValueToApply = vectorOfAngle.y; break;
	case eZAxis:				ValueToApply = vectorOfAngle.z; break;
	default:
		break;
	}
}

bool	
CSteamAudioEmitter::Process(fr_f32** ppData, fr_i32 Frames)
{
	if (!pParentListener) return false;
	fr_i32 BaseEmitterPosition = 0;
	fr_i32 BaseListenerPosition = 0;
	fr_i32 FramesReaded = 0;
	IMediaListener* ThisListener = (IMediaListener*)pParentListener;

	if (EmittersState == eStopState || EmittersState == ePauseState) return false;

	BaseEmitterPosition = (fr_i32)GetPosition();
	BaseListenerPosition = (fr_i32)ThisListener->GetPosition();

	/* Set emitter position to listener and read data */
	ThisListener->SetPosition((fr_i64)BaseEmitterPosition);
	FramesReaded = ThisListener->Process(ppData, Frames);
	if (FramesReaded < Frames) {
		BaseEmitterPosition = 0;
		ThisListener->SetPosition((fr_i64)BaseEmitterPosition);

		/* We don't want replay audio if we set this flag */
		if (EmittersState == ePlayState) {
			EmittersState = eStopState;
		} else {
			fr_f32* pTempData[MAX_CHANNELS] = {};
			for (size_t i = 0; i < ListenerFormat.Channels; i++) {
				pTempData[i] = &ppData[i][FramesReaded];
			}

			FramesReaded = ThisListener->Process(pTempData, Frames - FramesReaded);
			BaseEmitterPosition = ThisListener->GetPosition();
		}
	}
	else {
		BaseEmitterPosition += FramesReaded;
	}

	/* Process by emitter effect */
	for (size_t i = 0; i < outputFormat.Channels; i++) {
		memcpy(InputBuffer.deinterleavedBuffer[i], ppData[i], sizeof(fr_f32) * Frames);
	}
	iplApplyBinauralEffect(BinauralEffect, BinauralRender, InputBuffer, vectorOfAngle, IPL_HRTFINTERPOLATION_NEAREST, OutputBuffer);
	for (size_t i = 0; i < outputFormat.Channels; i++) {
		memcpy(ppData[i], OutputBuffer.deinterleavedBuffer[i], sizeof(fr_f32) * Frames);
	}

	EffectNodeStruct* pEffectToProcess = pFirstEffect;
	while (pEffectToProcess) {
		pEffectToProcess->pEffect->Process(ppData, Frames);
		pEffectToProcess = pEffectToProcess->pNext;
	}

	SetPosition(BaseEmitterPosition);
	ThisListener->SetPosition((fr_i64)BaseListenerPosition);
	return true;
}
#else
IBaseEmitter*
GetSteamEmitter()
{
	return nullptr;
}
#endif