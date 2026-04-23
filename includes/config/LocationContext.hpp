#ifndef LOCATIONCONTEXT_HPP
# define LOCATIONCONTEXT_HPP

# include <iostream>
# include <string>
# include <vector>

class LocationContext {
	private:
		std::string _path;

	public:
		LocationContext();
		~LocationContext();
		LocationContext(const LocationContext& other);
		LocationContext& operator=(const LocationContext& other);

		void setPath(const std::string& path);
		const std::string& getPath() const;
};

#endif
