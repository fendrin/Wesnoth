#include <SDL.h>
#include <SDL_rwops.h>

#include "filesystem.hpp"
#include "log.hpp"

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

namespace filesystem {

#if SDL_VERSION_ATLEAST(2,0,0)
static Sint64 ifs_size (struct SDL_RWops * context);
static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence);
static size_t SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum);
static size_t SDLCALL ifs_write(struct SDL_RWops *context, const void *ptr, size_t size, size_t num);
static int SDLCALL ifs_close(struct SDL_RWops *context);
#else
static int SDLCALL ifs_seek(struct SDL_RWops *context, int offset, int whence);
static int SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, int size, int maxnum);
static int SDLCALL ifs_write(struct SDL_RWops *context, const void *ptr, int size, int num);
static int SDLCALL ifs_close(struct SDL_RWops *context);
#endif

SDL_RWops* load_RWops(const std::string &path) {
	SDL_RWops *rw = SDL_AllocRW();

#if SDL_VERSION_ATLEAST(2,0,0)
	rw->size = &ifs_size;
#endif
	rw->seek = &ifs_seek;
	rw->read = &ifs_read;
	rw->write = &ifs_write;
	rw->close = &ifs_close;

	rw->type = 7; // Random number that is larger than 5

	std::istream *ifs = istream_file(path);
	if(!ifs) {
		ERR_FS << "load_RWops: istream_file returned NULL on " << path << '\n';
		return NULL;
	}

	rw->hidden.unknown.data1 = ifs;

	return rw;
}


#if SDL_VERSION_ATLEAST(2,0,0)
static Sint64 ifs_size (struct SDL_RWops * context) {
	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
	std::streampos orig = ifs->tellg();

	ifs->seekg(0, std::ios::end);

	std::streampos len = ifs->tellg();

	ifs->seekg(orig);

	return len;

}
#endif

#if SDL_VERSION_ATLEAST(2,0,0)
static Sint64 SDLCALL ifs_seek(struct SDL_RWops *context, Sint64 offset, int whence) {
#else
static int SDLCALL ifs_seek(struct SDL_RWops *context, int offset, int whence) {
#endif

	std::ios_base::seekdir seekdir;
	switch(whence){
	case RW_SEEK_SET:
		seekdir = std::ios_base::beg;
		if(offset < 0)
			offset = 0;
		break;
	case RW_SEEK_CUR:
		seekdir = std::ios_base::cur;
		break;
	case RW_SEEK_END:
		seekdir = std::ios_base::end;
		if(offset > 0)
			offset = 0;
		break;
	default:
		assert(false);
		throw "assertion ignored";
	}
	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
	const std::ios_base::iostate saved_state = ifs->rdstate();

	ifs->seekg(offset, seekdir);

	if(saved_state != ifs->rdstate() && offset < 0) {
		ifs->clear(saved_state);
		ifs->seekg(0, std::ios_base::beg);
	}

	std::streamsize pos = ifs->tellg();
	return static_cast<int>(pos);
}

#if SDL_VERSION_ATLEAST(2,0,0)
static size_t SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
#else
static int SDLCALL ifs_read(struct SDL_RWops *context, void *ptr, int size, int maxnum) {
#endif
	std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);

	// This seems overly simplistic, but it's the same as mem_read's implementation
	ifs->read(static_cast<char*>(ptr), maxnum * size);
	std::streamsize num = ifs->good() ? maxnum : ifs->gcount() / size;

	// EOF sticks unless we clear it. Bad is an actual I/O error
	if(!ifs->bad())
		ifs->clear();

	return static_cast<int>(num);
}

#if SDL_VERSION_ATLEAST(2,0,0)
static size_t SDLCALL ifs_write(struct SDL_RWops * /*context*/, const void * /*ptr*/, size_t /*size*/, size_t /*num*/) {
#else
static int SDLCALL ifs_write(struct SDL_RWops * /*context*/, const void * /*ptr*/, int /*size*/, int /*num*/) {
#endif
	SDL_SetError("Writing not implemented");
	return 0;
}
static int SDLCALL ifs_close(struct SDL_RWops *context) {
	if (context) {
		std::istream *ifs = static_cast<std::istream*>(context->hidden.unknown.data1);
		delete ifs;
		SDL_FreeRW(context);
	}
	return 0;
}

}
