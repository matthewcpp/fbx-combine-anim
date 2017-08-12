#include <iostream>
#include <fbxsdk.h>
#include <cassert>
#include <fstream>
#include <vector>

FbxScene* CreateSceneFromFile(const char* fileName, FbxManager* fbxManager);
void AppendAnimations(FbxScene* destScene, FbxScene* srcScene);
void DebugAnimInfo(FbxScene* scene);
void DebugKeys(FbxAnimLayer* layer, FbxNode* node, std::ofstream& debugLog);
double DetermineAnimTime(FbxScene* scene, int stack);

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

	//DebugAnimInfo(masterScene);

	FbxExporter* exporter = FbxExporter::Create(fbxManager, "");
	exporter->Initialize("test_out.fbx");
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



void DebugAnimInfo(FbxScene* scene) {
	FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(0);
	FbxTime t = animStack->LocalStop.Get();

	std::ofstream debugLog("log.txt");

	int srcAnimLayerCount = animStack->GetSrcObjectCount<FbxAnimLayer>();
	for (int l = 0; l < srcAnimLayerCount; l++) {
		FbxAnimLayer* srcLayer = animStack->GetSrcObject<FbxAnimLayer>(l);

		DebugKeys(srcLayer, scene->GetRootNode(), debugLog);
	}
}

void DebugCurve(FbxAnimCurveNode* srcAnimCurveNode, std::ofstream& debugLog) {
	if (srcAnimCurveNode) {
		int channelCount = srcAnimCurveNode->GetChannelsCount();
		assert(channelCount == 3);

		FbxAnimCurve* xCurve = srcAnimCurveNode->GetCurve(0);
		FbxAnimCurve* yCurve = srcAnimCurveNode->GetCurve(1);
		FbxAnimCurve* zCurve = srcAnimCurveNode->GetCurve(2);

		if (xCurve != nullptr && yCurve != nullptr && yCurve != nullptr) {
			int keyCount = xCurve->KeyGetCount();
			for (int i = 0; i < keyCount; i++) {
				FbxAnimCurveKey xKey = xCurve->KeyGet(i);
				FbxAnimCurveKey yKey = yCurve->KeyGet(i);
				FbxAnimCurveKey zKey = zCurve->KeyGet(i);

				FbxTime time = xKey.GetTime();

				debugLog << "key " << i << ": " << xKey.GetValue() << ", " << yKey.GetValue() << ", " << zKey.GetValue() << "\tt:" << time.GetSecondDouble() << std::endl;
			}
		}
	}
}

void DebugKeys(FbxAnimLayer* layer, FbxNode* node, std::ofstream& debugLog) {
	FbxAnimCurveNode* srcAnimCurveNode = node->LclRotation.GetCurveNode(layer);

	debugLog << node->GetName() << std::endl;

	debugLog << "Translate:" << std::endl;
	DebugCurve(node->LclTranslation.GetCurveNode(layer), debugLog);

	debugLog << "Rotate:" << std::endl;
	DebugCurve(node->LclRotation.GetCurveNode(layer), debugLog);

	debugLog << "Scale:" << std::endl;
	DebugCurve(node->LclScaling.GetCurveNode(layer), debugLog);

	int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		DebugKeys(layer, node->GetChild(i), debugLog);
	}
}


double DetermineAnimTime(FbxScene* scene, int stack) {
	FbxAnimLayer* layer = scene->GetSrcObject<FbxAnimStack>(stack)->GetSrcObject<FbxAnimLayer>(0);

	std::vector<FbxNode*> nodes;
	nodes.push_back(scene->GetRootNode());

	do {
		FbxNode* node = nodes.back();
		nodes.pop_back();

		FbxAnimCurve* curve = node->LclTranslation.GetCurveNode(layer)->GetCurve(0);
		if (curve) {
			return curve->KeyGetTime(curve->KeyGetCount() - 1).GetSecondDouble();
		}

		curve = node->LclRotation.GetCurveNode(layer)->GetCurve(0);
		if (curve) {
			return curve->KeyGetTime(curve->KeyGetCount() - 1).GetSecondDouble();
		}

		curve = node->LclScaling.GetCurveNode(layer)->GetCurve(0);
		if (curve) {
			return curve->KeyGetTime(curve->KeyGetCount() - 1).GetSecondDouble();
		}

		int childCount = node->GetChildCount();
		for (int i = 0; i < childCount; i++) {
			nodes.push_back(node->GetChild(i));
		}
	} while (nodes.size() > 0);


	return 0.0;
}

void AddTime(FbxTime& fbxTime, double seconds) {
	FbxTime t;
	t.SetSecondDouble(seconds);

	fbxTime += t;
}