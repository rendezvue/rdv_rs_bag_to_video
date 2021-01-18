#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <librealsense2/rsutil.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <opencv2/opencv.hpp>   

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams
#include <algorithm>
#include <cstring>

#if __has_include(<filesystem>)
#include <filesystem>
namespace filesystem = std::filesystem;
#else
#include <experimental/filesystem>
#if _WIN32
namespace filesystem = std::experimental::filesystem::v1;
#else
namespace filesystem = std::experimental::filesystem;
#endif
#endif

int main(int argc, char * argv[])
{
	rs2::pipeline pipeline;
	rs2::pipeline_profile pipeline_profile;
	filesystem::path bag_file;

	bool b_write_image = false ;
	bool b_write_video = false ;
	
	//parameter
	if(argc<2)
    {
        printf("Too few arguments\n");
		return 0 ;
    }
    else
    {
        namespace po = boost::program_options;
        po::options_description desc("Options");
        desc.add_options()
			("i", "")
            ("v", "")
            ("bag", po::value<std::string>()->default_value(""));

        po::variables_map vm;
        po::store(po::parse_command_line(argc,argv,desc),vm);
        po::notify(vm);

		std::cout << "i parsed: "  << vm.count("i") << "\n";
        std::cout << "v parsed: "  << vm.count("v") << "\n";
        std::cout << "bag: '" << boost::any_cast<std::string>(vm["bag"].value()) << "'\n";

		bag_file = boost::any_cast<std::string>(vm["bag"].value()) ;
		if( vm.count("i") > 0 )	b_write_image = true ;
		if( vm.count("v") > 0 )	b_write_video = true ;
    }

	//-------------------------------------------
	// Initialize Sensor
	{
		rs2::config config;
	    rs2::context context;
	    const rs2::playback playback = context.load_device( bag_file.string() );
	    const std::vector<rs2::sensor> sensors = playback.query_sensors();
	    for( const rs2::sensor& sensor : sensors ){
	        const std::vector<rs2::stream_profile> stream_profiles = sensor.get_stream_profiles();
	        for( const rs2::stream_profile& stream_profile : stream_profiles ){
	            config.enable_stream( stream_profile.stream_type(), stream_profile.stream_index() );
	        }
	    }

	    // Start Pipeline
	    config.enable_device_from_file( playback.file_name() );
	    pipeline_profile = pipeline.start( config );

	    // Set Non Real Time Playback
	    pipeline_profile.get_device().as<rs2::playback>().set_real_time( false );

	    // Show Enable Streams
	    const std::vector<rs2::stream_profile> stream_profiles = pipeline_profile.get_streams();
	    for( const rs2::stream_profile stream_profile : stream_profiles ){
	        std::cout << stream_profile.stream_name() << std::endl;
	    }
	}

	//----------------------------------
	// Initialize Save
	filesystem::path directory;
	{
	    // Create Root Directory (Bag File Name)
	    directory = bag_file.parent_path().generic_string() + "/" + bag_file.stem().string();
	    if( !filesystem::create_directories( directory ) ){
	        //throw std::runtime_error( "failed can't create root directory" );
	        printf( "exist root directory\n" );
	    }
		else
		{

		    // Create Sub Directory for Each Streams (Stream Name)
		    const std::vector<rs2::stream_profile> stream_profiles = pipeline_profile.get_streams();
		    for( const rs2::stream_profile stream_profile : stream_profiles ){
		        filesystem::path sub_directory = directory.generic_string() + "/" + stream_profile.stream_name();
		        filesystem::create_directories( sub_directory );
		    }

			//Create Video Directory
			filesystem::path vid_directory = directory.generic_string() + "/Video";
	        filesystem::create_directories( vid_directory );
		}
	}

	//Run

	rs2::frameset frameset;
	
	// Color Buffer
    rs2::frame color_frame;
    cv::Mat color_mat;
    uint32_t color_width;
    uint32_t color_height;

    // Depth Buffer
    rs2::frame depth_frame;
    cv::Mat depth_mat;
    uint32_t depth_width;
    uint32_t depth_height;

	// Retrieve Last Position
    uint64_t last_position = pipeline_profile.get_device().as<rs2::playback>().get_position();

	//video writer
	cv::VideoWriter *pVideoWriter = NULL ;
	
	while(1)
	{
		//------------------------------------------------------
		// Update Data
		// Update Frame
    	frameset = pipeline.wait_for_frames();

		
		//Color
		color_frame = frameset.get_color_frame();
	    if( color_frame )
		{
	        // Retrive Frame Size
		    color_width = color_frame.as<rs2::video_frame>().get_width();
		    color_height = color_frame.as<rs2::video_frame>().get_height();
	    }
		
		//Depth
	    depth_frame = frameset.get_depth_frame();
	    if( depth_frame )
		{
	        // Retrive Frame Size
		    depth_width = depth_frame.as<rs2::video_frame>().get_width();
		    depth_height = depth_frame.as<rs2::video_frame>().get_height();
	    }
		// Update Data
		//------------------------------------------------------

		//------------------------------------------------------
		//Make RS Frame to Mat
		if( color_frame )
		{
		    // Create cv::Mat form Color Frame
		    const rs2_format color_format = color_frame.get_profile().format();
		    switch( color_format ){
		        // RGB8
		        case rs2_format::RS2_FORMAT_RGB8:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_8UC3, const_cast<void*>( color_frame.get_data() ) ).clone();
		            cv::cvtColor( color_mat, color_mat, cv::COLOR_RGB2BGR );
		            break;
		        }
		        // RGBA8
		        case rs2_format::RS2_FORMAT_RGBA8:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_8UC4, const_cast<void*>( color_frame.get_data() ) ).clone();
		            cv::cvtColor( color_mat, color_mat, cv::COLOR_RGBA2BGRA );
		            break;
		        }
		        // BGR8
		        case rs2_format::RS2_FORMAT_BGR8:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_8UC3, const_cast<void*>( color_frame.get_data() ) ).clone();
		            break;
		        }
		        // BGRA8
		        case rs2_format::RS2_FORMAT_BGRA8:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_8UC4, const_cast<void*>( color_frame.get_data() ) ).clone();
		            break;
		        }
		        // Y16 (GrayScale)
		        case rs2_format::RS2_FORMAT_Y16:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_16UC1, const_cast<void*>( color_frame.get_data() ) ).clone();
		            constexpr double scaling = static_cast<double>( std::numeric_limits<uint8_t>::max() ) / static_cast<double>( std::numeric_limits<uint16_t>::max() );
		            color_mat.convertTo( color_mat, CV_8U, scaling );
		            break;
		        }
		        // YUYV
		        case rs2_format::RS2_FORMAT_YUYV:
		        {
		            color_mat = cv::Mat( color_height, color_width, CV_8UC2, const_cast<void*>( color_frame.get_data() ) ).clone();
		            cv::cvtColor( color_mat, color_mat, cv::COLOR_YUV2BGR_YUYV );
		            break;
		        }
		        default:
		            throw std::runtime_error( "unknown color format" );
		            break;
		    }
		}

		if( depth_frame )
		{
		    // Create cv::Mat form Depth Frame
		    depth_mat = cv::Mat( depth_height, depth_width, CV_16UC1, const_cast<void*>( depth_frame.get_data() ) ).clone();
		}
		//Make RS Frame to Mat
		//------------------------------------------------------

		//------------------------------------------------------
		//Save
		//Color
		if( b_write_image )
		{
			if( color_frame && !color_mat.empty() )
			{
			    // Create Save Directory and File Name
			    std::ostringstream oss;
			    oss << directory.generic_string() << "/Color/";
			    oss << std::setfill( '0' ) << std::setw( 6 ) << color_frame.get_frame_number() << ".png";

			    // Write Color Image
			    cv::imwrite( oss.str(), color_mat);
			}

			//Depth
			if( depth_frame && !depth_mat.empty() )
			{
				const bool scaling = true ;
			    // Create Save Directory and File Name
			    std::ostringstream oss;
			    oss << directory.generic_string() << "/Depth/";
			    oss << std::setfill( '0' ) << std::setw( 6 ) << depth_frame.get_frame_number() << ".png";

			    // Scaling
			    cv::Mat scale_mat = depth_mat;
			    if( scaling ){
			        depth_mat.convertTo( scale_mat, CV_8U, -255.0 / 10000.0, 255.0 ); // 0-10000 -> 255(white)-0(black)
			    }

			    // Write Depth Image
			    cv::imwrite( oss.str(), scale_mat );
			}
		}

		if( b_write_video )
		{
			if( pVideoWriter == NULL )
			{	
				 //video writer
				 cv::Size frameSize(static_cast<int>(color_width), static_cast<int>(color_height));
				 std::string str_path_video = directory.generic_string() + "/Video/video.mp4"; 
				 pVideoWriter = new cv::VideoWriter(str_path_video, cv::VideoWriter::fourcc('M','P','4','V'), 20, frameSize, true); //initialize the VideoWriter object 
				
				 if ( !pVideoWriter->isOpened() ) //if not initialize the VideoWriter successfully, exit the program
				{
					 std::cout << "ERROR: Failed to write the video" << std::endl;
				}		
			}
			else
			{
				if( color_frame && !color_mat.empty() )
				{
					//write
					//받아온 Frame을 저장한다.
					(*pVideoWriter) << color_mat;
				}
			}
		}
		//Save
		//------------------------------------------------------

        // Key Check
        const int32_t key = cv::waitKey( 1 );
        if( key == 'q' ){
            break;
        }

        // End of Position
        const uint64_t current_position = pipeline_profile.get_device().as<rs2::playback>().get_position();
        if( static_cast<int64_t>( current_position - last_position ) < 0 ){
            break;
        }
        last_position = current_position;
	}

	//Exit
	if( pVideoWriter ) delete pVideoWriter ;
	
	// Stop Pipline
    pipeline.stop();

    return EXIT_SUCCESS;
}
