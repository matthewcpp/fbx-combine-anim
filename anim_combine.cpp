#include <iostream>

#include <fbxsdk.h>

FbxScene* CreateSceneFromFile(const char* fileName, FbxManager* fbxManager);
void AppendAnimations(FbxScene* destScene, FbxScene* srcScene);

int main(int argc, char** argv) {

	FbxManager* fbxManager = FbxManager::Create();
	FbxScene* masterScene = nullptr;

	FbxTime::SetGlobalTimeMode(FbxTime::eFrames60);
	

	for (int i = 1; i < argc; i++) {
		std::cout << "Loading Scene: " << argv[i] << std::endl;
		FbxScene* scene = CreateSceneFromFile(argv[i], fbxManager);

		if (!masterScene) {
			masterScene = scene;
		}
		else {
			AppendAnimations(masterScene, scene);
		}
	}

	FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	ioSettings->SetBoolProp(EXP_FBX_EMBEDDED, true);

	FbxExporter* exporter = FbxExporter::Create(fbxManager, "");
	exporter->Initialize("test_out.fbx", -1, ioSettings);
	exporter->SetFileExportVersion(FBX_2014_00_COMPATIBLE);
	exporter->Export(masterScene);
	exporter->Destroy();
		
	fbxManager->Destroy();

	return 0;
}

FbxScene* CreateSceneFromFile(const char* fileName, FbxManager* fbxManager) {
	FbxScene* scene = FbxScene::Create(fbxManager, fileName);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	const bool result = importer->Initialize(fileName, -1, fbxManager->GetIOSettings());

	if (result) {
		importer->Import(scene);
	}
	else {
		FbxString error = importer->GetStatus().GetErrorString();
		FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", error.Buffer());
	}

	importer->Destroy();

	return scene;
}

void CopyKeys(FbxAnimCurve* destCurve, FbxAnimCurve* srcCurve) {
	destCurve->KeyModifyBegin();

	int srcKeyCount = srcCurve->KeyGetCount();

	for (int i = 0; i < srcKeyCount; i++) {
		int keyIndex = destCurve->KeyAdd(srcCurve->KeyGetTime(i));

		destCurve->KeySetInterpolation(keyIndex, srcCurve->KeyGetInterpolation(i));
		destCurve->KeySetValue(keyIndex, srcCurve->KeyGetValue(i));
	}

	destCurve->KeyModifyEnd();
}

void AppendCurves(FbxAnimLayer* destlayer, FbxNode* destNode, FbxAnimLayer* srclayer, FbxNode* srcNode) {
	FbxAnimCurve* srcCurveX = srcNode->LclTranslation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_X, false);
	FbxAnimCurve* srcCurveY = srcNode->LclTranslation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
	FbxAnimCurve* srcCurveZ = srcNode->LclTranslation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

	if (srcCurveX) {
		FbxAnimCurve* destCurveX = destNode->LclTranslation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* destCurveY = destNode->LclTranslation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* destCurveZ = destNode->LclTranslation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		CopyKeys(destCurveX, srcCurveX);
		CopyKeys(destCurveY, srcCurveY);
		CopyKeys(destCurveZ, srcCurveZ);

	}

	srcCurveX = srcNode->LclRotation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_X, false);
	srcCurveY = srcNode->LclRotation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
	srcCurveZ = srcNode->LclRotation.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

	if (srcCurveX) {
		FbxAnimCurve* destCurveX = destNode->LclRotation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* destCurveY = destNode->LclRotation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* destCurveZ = destNode->LclRotation.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		CopyKeys(destCurveX, srcCurveX);
		CopyKeys(destCurveY, srcCurveY);
		CopyKeys(destCurveZ, srcCurveZ);

	}

	srcCurveX = srcNode->LclScaling.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_X, false);
	srcCurveY = srcNode->LclScaling.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Y, false);
	srcCurveZ = srcNode->LclScaling.GetCurve(srclayer, FBXSDK_CURVENODE_COMPONENT_Z, false);

	if (srcCurveX) {
		FbxAnimCurve* destCurveX = destNode->LclScaling.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* destCurveY = destNode->LclScaling.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* destCurveZ = destNode->LclScaling.GetCurve(destlayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		CopyKeys(destCurveX, srcCurveX);
		CopyKeys(destCurveY, srcCurveY);
		CopyKeys(destCurveZ, srcCurveZ);

	}


	int childCount = srcNode->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		AppendCurves(destlayer, destNode->GetChild(i), srclayer, srcNode->GetChild(i));
	}
	
}

void AppendAnimations(FbxScene* destScene, FbxScene* srcScene) {
	int srcStackCount = srcScene->GetSrcObjectCount<FbxAnimStack>();
	for (int i = 0; i < srcStackCount; i++) {
		FbxAnimStack* srcStack = srcScene->GetSrcObject<FbxAnimStack>(i);
		FbxAnimLayer* srcLayer = srcStack->GetSrcObject<FbxAnimLayer>(0);

		destScene->CreateAnimStack(srcStack->GetName());
		FbxAnimStack* destStack = destScene->GetSrcObject<FbxAnimStack>(destScene->GetSrcObjectCount<FbxAnimStack>() -1);

		FbxAnimLayer* destLayer = FbxAnimLayer::Create(destScene, srcLayer->GetName());
		destStack->AddMember(destLayer);

		AppendCurves(destLayer, destScene->GetRootNode(), srcLayer, srcScene->GetRootNode());

		destStack->LocalStop.Set(srcStack->LocalStop.Get());
	}	
}
