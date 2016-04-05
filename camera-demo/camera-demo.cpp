#include <iostream>
#include <memory>
#include <chrono>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "CameraDetector.h"
#include "AffdexException.h"

#include "AFaceListener.hpp"
#include "PlottingImageListener.hpp"
#include "StatusListener.hpp"

using namespace std;
using namespace affdex;

int main(int argsc, char ** argsv)
{
    // Parse and check the data folder (with assets)
#ifdef _WIN32
    affdex::path DATA_FOLDER = L"data";
    affdex::path LICENSE_PATH = L"test.license";
#else // _WIN32
    affdex::path DATA_FOLDER = "data";
    affdex::path LICENSE_PATH = "test.license";
#endif // _WIN32
    int process_framerate = 30;
    int camera_framerate = 15;
    int camera_id = 0;
    unsigned int nFaces = 1;
    bool draw_display = true;
    int faceDetectorMode = (int)FaceDetectorMode::LARGE_FACES;
    
    const int precision = 2;
    std::cerr.precision(precision);
    std::cout.precision(precision);
    
    namespace po = boost::program_options; // abbreviate namespace
    po::options_description description("Project for demoing the Windows SDK CameraDetector class (grabbing and processing frames from the camera).");
    description.add_options()
    ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
    ("data,d", po::wvalue< affdex::path >(&DATA_FOLDER)->default_value(affdex::path(L"data"), std::string("data")), "Path to the data folder")
    ("license,l", po::wvalue< affdex::path >(&LICENSE_PATH)->default_value(affdex::path(L"test.license"), std::string("test.license")), "License file.")
#else //  _WIN32
    ("data,d", po::value< affdex::path >(&DATA_FOLDER)->default_value(affdex::path("data"), std::string("data")), "Path to the data folder")
    ("license,l", po::value< affdex::path >(&LICENSE_PATH)->default_value(affdex::path("test.license"), std::string("test.license")), "License file.")
#endif // _WIN32
    ("pfps", po::value< int >(&process_framerate)->default_value(30), "Processing framerate.")
    ("cfps", po::value< int >(&camera_framerate)->default_value(30), "Camera capture framerate.")
    ("cid", po::value< int >(&camera_id)->default_value(0), "Camera ID.")
    ("faceMode", po::value< int >(&faceDetectorMode)->default_value((int) FaceDetectorMode::LARGE_FACES), "Face detector mode (large faces vs small faces).")
    ("numFaces", po::value< unsigned int >(&nFaces)->default_value(1), "Number of faces to be tracked.")
    ("draw", po::value< bool >(&draw_display)->default_value(true), "Draw metrics on screen.")
    ;
    
    po::variables_map args;
    try
    {
        po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
        if (args["help"].as<bool>())
        {
            std::cout << description << std::endl;
            return 0;
        }
        po::notify(args);
    }
    catch (po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << "For help, use the -h option." << std::endl << std::endl;
        return 1;
    }
    
    if (!boost::filesystem::exists(DATA_FOLDER))
    {
        std::cerr << "Data folder doesn't exist: " << std::string(DATA_FOLDER.begin(), DATA_FOLDER.end()) << std::endl;
        std::cerr << "Try specifying the folder through the command line:\n" << description << std::endl;
        return 1;
    }
    if (!boost::filesystem::exists(LICENSE_PATH))
    {
        std::cerr << "License file doesn't exist: " << std::string(LICENSE_PATH.begin(), LICENSE_PATH.end()) << std::endl;
        std::cerr << "Try specifying the folder through the command line:\n" << description << std::endl;
        return 1;
    }
    
    try
    {
        std::ofstream csvFileStream("output.csv");
        
        CameraDetector cameraDetector(camera_id, camera_framerate, process_framerate, nFaces, (FaceDetectorMode) faceDetectorMode);
        shared_ptr<PlottingImageListener> listenPtr(new PlottingImageListener(csvFileStream, draw_display));
        shared_ptr<StatusListener> videoListenPtr(new StatusListener());
        
        std::cout << "Max num of faces set to: " << cameraDetector.getMaxNumberFaces() << std::endl;
        std::string mode;
        switch (cameraDetector.getFaceDetectorMode())
        {
            case FaceDetectorMode::LARGE_FACES:
                mode = "LARGE_FACES";
                break;
            case FaceDetectorMode::SMALL_FACES:
                mode = "SMALL_FACES";
                break;
            default:
                break;
        }
        std::cout << "Face detector mode set to: " << mode << std::endl;
        
        //Expressions
        cameraDetector.setDetectAttention(true);
        cameraDetector.setDetectSmile(true);
        cameraDetector.setDetectChinRaise(true);
        cameraDetector.setDetectLipPress(true);
        cameraDetector.setDetectInnerBrowRaise(true);
        cameraDetector.setDetectLipPucker(true);
        cameraDetector.setDetectLipCornerDepressor(true);
        cameraDetector.setDetectUpperLipRaise(true);
        cameraDetector.setDetectLipSuck(true);
        cameraDetector.setDetectMouthOpen(true);
        cameraDetector.setDetectNoseWrinkle(true);
        cameraDetector.setDetectBrowFurrow(true);
        cameraDetector.setDetectBrowRaise(true);
        cameraDetector.setDetectEyeClosure(true);
        cameraDetector.setDetectSmirk(true);
        //Emotions
        cameraDetector.setDetectEngagement(true);
        cameraDetector.setDetectValence(true);
        cameraDetector.setDetectAnger(true);
        cameraDetector.setDetectDisgust(true);
        cameraDetector.setDetectJoy(true);
        cameraDetector.setDetectSadness(true);
        cameraDetector.setDetectSurprise(true);
        cameraDetector.setDetectContempt(true);
        
        //Appearance
        cameraDetector.setDetectGender(true);
        cameraDetector.setDetectGlasses(true);
        
        cameraDetector.setDetectAllEmojis(true);
        
        cameraDetector.setLicensePath(LICENSE_PATH);
        cameraDetector.setClassifierPath(DATA_FOLDER);
        cameraDetector.setImageListener(listenPtr.get());
        cameraDetector.setProcessStatusListener(videoListenPtr.get());
        cameraDetector.start();
#ifdef _WIN32
        while (!GetAsyncKeyState(VK_ESCAPE) && videoListenPtr->isRunning())
#else //  _WIN32
        while (videoListenPtr->isRunning())
#endif
        {
            if (listenPtr->getDataSize() > 0)
            {
                    
                std::pair<Frame, std::map<FaceId, Face> > dataPoint = listenPtr->getData();
                Frame frame = dataPoint.first;
                std::map<FaceId, Face> faces = dataPoint.second;
                    
                    
                if (draw_display)
                {
                    listenPtr->draw(faces, frame);
                }
                    
                std::cerr << "timestamp: " << frame.getTimestamp()
                          << " cfps: " << listenPtr->getCaptureFrameRate()
                          << " pfps: " << listenPtr->getProcessingFrameRate()
                          << " faces: "<< faces.size() << endl;
                    
                    listenPtr->outputToFile(faces, frame.getTimestamp());
            }
        }
        cameraDetector.stop();
    }
    catch (AffdexException ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    
    return 0;
}
