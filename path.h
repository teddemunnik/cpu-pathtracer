#pragma once
#include <string>

namespace tt{
//Class for handing File/Directory paths
class Path{
private:
	std::string m_Data;
public:
	Path(){}
	explicit Path(const std::string& data) :
		m_Data(data){}

	//Returns the underlying string
	std::string& data(){
		return m_Data;
	}
	//Returns the underlying string
	const std::string& data() const{
		return m_Data;
	}
	//Checks if this is an absolute path (has drive letter)
	bool isAbsolute() const{
		//Assumption: Is drive letter always 1 character?
		return	(m_Data.compare(1, 2, ":\\") == 0 || m_Data.compare(1, 2, ":/") == 0);
	}
	//Checks if this is a relative path (doesn't have drive letter)
	bool isRelative() const{
		return !isAbsolute();
	}
	//Checks if this is a file path
	bool isFilePath() const{
		//Find the lastest . (extension)
		const std::string::size_type lastDot = m_Data.find_last_of('.');
		const std::string::size_type lastSlash = m_Data.find_last_of("/\\");
		if(lastDot != std::string::npos){
			if(lastSlash == std::string::npos){
				return true;
			}else{
				return lastSlash < lastDot; 
			}
		}else{
			return false;
		}
	}

	//Checks if this is a directory path (not a file path)
	bool isDirectoryPath() const{
		return !isFilePath();
	}

	//Returns the root path of the path
	//If the path is absolute the drive letter is returned (i.e. C:/)
	//If the path is relative to the current drive (\), \ is returned
	//Else an empty path is returned
	Path rootPath() const{
		if(m_Data.empty()) return Path();
		if(m_Data.compare(1, 2, ":\\") == 0 || m_Data.compare(1, 2, ":/") == 0){
			return Path(m_Data.substr(0, 3));
		}
		if(m_Data[0] == '\\' || m_Data[0] == '/'){
			return Path(m_Data.substr(0, 1));
		}
		return Path();
	}
	
	//Returns the path to the directory of the path (without file name/extension)
	//Guarantees a trailing slash
	Path directoryPath() const{
		Path ret;
		if(m_Data.length() == 0) return ret;

		if(isDirectoryPath()){
			//This is a directory path, append trailing \ if neccesary
			ret.m_Data = m_Data;
			if(m_Data.back() != '/' && m_Data.back() != '\\'){
				ret.m_Data.append("\\");
			}
		}else{
			//Remove file part
			const std::string::size_type lastslash = m_Data.find_last_of("/\\");
			ret.m_Data = m_Data.substr(0, lastslash+1);
		}
		return ret;
	}

	//Appends two paths
	//If other is an absolute path, or relative to the current directory (.) the path will be set to other
	//If other is relative to the current drive, the drive will be taken from this path
	//Else the two paths are combined
	static Path append(const Path& lhs, const Path& rhs){
		Path ret;
		if(rhs.isRelative()){
			if(rhs.m_Data.length() >= 1 && (rhs.m_Data[0] == '\\' || rhs.m_Data[0] == '/')){ //Relative to current drive
				ret.m_Data = lhs.rootPath().m_Data + rhs.m_Data.substr(1);
			}
			else if(rhs.m_Data.length() >= 1 && rhs.m_Data[0] == '.' && (rhs.m_Data.length() < 2 || rhs.m_Data[1] != '.')){ //relative to current directory
				ret = rhs;
			}else{ 
				//Concatenate the two paths
				Path directoryPath = lhs.directoryPath();

				std::string::size_type it = 0;
				std::string::size_type dir_it=directoryPath.m_Data.length()-1;
				while(rhs.m_Data.compare(it, 3, "../") == 0|| rhs.m_Data.compare(it, 3, "..\\") == 0){
					//Go back a folder on the left size (if possible)
					if(dir_it > 0){
						std::string::size_type res = directoryPath.m_Data.find_last_of("/\\", dir_it-1);
						if(res != std::string::npos){
							dir_it = res;
						}
					}
					it += 3;
				}
				ret.m_Data = directoryPath.m_Data.substr(0, dir_it+1);
				ret.m_Data.append(rhs.m_Data.substr(it));
			}
		}else{
			ret = rhs;
		}
		return ret;
	}

	//Converts all directory seperators to prefered type (\ for windows)
	void clean(){
		//convert directory seperators
		std::string::size_type cur = m_Data.find_first_of('/');
		while(cur != std::string::npos){
			m_Data[cur] = '\\';
			cur = m_Data.find_first_of('/', cur+1);
		}
	}
};
};//namespace tt