#include <tuple>
#include <cmath>
#include <algorithm>
#include "MIDITrack.h"


size_t MIDITrack::readTrack(const std::vector<char>& buffer, size_t pos){
	const size_t backupPos = pos;
	
	//Check header
	if( !(buffer[pos] == 'M' && buffer[pos+1] == 'T' && buffer[pos+2] == 'r' && buffer[pos+3] == 'k')){
		std::cerr << "[ERROR]: Missing track." << std::endl;
		return 3;
	}
	pos += 4;
	uint32_t length = read32(buffer, pos);
	pos += 4;
	
	if(length == 0){
		std::cerr << "[ERROR]: Empty track." << std::endl;
		return 3;
	}

	while(pos < backupPos + 8 + length){
		
		size_t delta = readVarLen(buffer,pos);
		uint8_t eventMetaType = read8(buffer, pos);
		
		if(eventMetaType == 0xFF){
			_events.push_back(MIDIEvent::readMetaEvent(buffer,pos, delta));
		} else if (eventMetaType >= 0xF0 && eventMetaType <= 0xF7){
			_events.push_back(MIDIEvent::readSysexEvent(buffer, pos, delta));
		}  else {
			_events.push_back(MIDIEvent::readMIDIEvent(buffer, pos, delta, _previousEventFirstByte));
		}
	}

	// Scan events for track info.
	// Could do it while creating events, but let's separate tasks, shall we?
	bool minorKey = false;
	short keyShift = 0;

	for(auto& event : _events){
		if(event.category == EventCategory::META){
			if(event.type == sequenceName){
				std::string tempName = "";
				for(size_t i = 0; i < event.data.size(); ++i){
					tempName += char(event.data[i]);
				}
				_name = tempName;
			} else if(event.type == instrumentName){
				std::string tempName = "";
				for(size_t i = 0; i < event.data.size(); ++i){
					tempName += char(event.data[i]);
				}
				_instrument = tempName;
			} else if (event.type == keySignature){
				keyShift = event.data[0];
				minorKey = (event.data[1] > 0);
			}
		}
	}
	
	std::cout << "[INFO]: Track " << _name << " (length: " << length << ", instrument: " << _instrument <<", " << (minorKey ? "minor": "major") << ")." << std::endl;
	
	return backupPos + 8 + length;
}

double MIDITrack::extractTempos(std::vector<MIDITempo> & tempos) const {
	size_t timeInUnits = 0;
	double signature = 4.0/4.0;
	for(auto& event : _events){
		timeInUnits += (event.delta);
		if(event.category == EventCategory::META && event.type == setTempo){
			const unsigned int tempo = ((event.data[0] & 0xFF) << 16) | ((event.data[1] & 0xFF) << 8) | (event.data[2] & 0xFF);
			tempos.emplace_back(timeInUnits, tempo);

		} else if(event.category == EventCategory::META && event.type == timeSignature){
			signature = double(event.data[0]) / double(std::pow(2,event.data[1]));

		}
	}
	return signature;
}

void MIDITrack::extractNotes(const std::vector<MIDITempo> & tempos, uint16_t unitsPerQuarterNote, short minId, short maxId){
	// Scan events, focusing on the note ON/OFF events.
	// Keep track of active notes.
	std::map<short, std::tuple<size_t, short, short>> currentNotes;
	size_t timeInUnits = 0;

	for(auto& event : _events){
		timeInUnits += (event.delta);
		if(event.category != EventCategory::MIDI || (event.type != noteOn && event.type != noteOff)){
			continue;
		}
		//Skip filtered notes.
		if(event.data[1] < minId || event.data[1] > maxId){
			continue;
		}
		// Shift note index.
		const size_t noteInd = event.data[1] - minId;
		if(currentNotes.count(noteInd) > 0){
			// The current note is already present.
			const auto & noteTuple = currentNotes[noteInd];
			// Finish it.
			const size_t start = std::get<0>(noteTuple);
			const size_t end = timeInUnits;
			// Create the final note with timing.
			// Look for the start and end timestamps using the tempos and their timestamps.

			const auto times = computeNoteTimings(tempos, start, end, unitsPerQuarterNote);

			const short velocity = std::get<1>(noteTuple);
			const short channel = std::get<2>(noteTuple);
			_notes.emplace_back(noteInd, times.first, times.second - times.first, velocity, channel);

			// Remove note.
			currentNotes.erase(noteInd);
		}

		// Check if we have to start a new note.
		const bool shouldNew = event.type == noteOn && event.data[2] > 0;
		if(shouldNew){
			currentNotes[noteInd] = std::make_tuple(timeInUnits, event.data[2], event.data[0]);
		}
	}
}

void MIDITrack::getNotes(std::vector<MIDINote> & notes, NoteType type) const {
	notes.clear();

	if(type == NoteType::MINOR){
		for(auto& note : _notes){
			if(noteIsMinor[note.note]){
				notes.push_back(note);
				notes.back().note = noteShift[note.note];
			}
		}
	} else if (type == NoteType::MAJOR){
		for(auto& note : _notes){
			if(!noteIsMinor[note.note]){
				notes.push_back(note);
				notes.back().note = noteShift[note.note];
			}
		}
	} else {
		for(auto& note : _notes){
			notes.push_back(note);
			notes.back().note = noteShift[note.note];
		}
	}
}

void MIDITrack::getNotesActive(std::vector<ActiveNoteInfos>& actives, double time) const {
	// Reset all notes.
	for(int i = 0; i < int(actives.size()); ++i){
		 actives[i].enabled = false;
	}
	const size_t count = _notes.size();
	for(size_t i = 0; i < count;++i){
		auto& note = _notes[i];
		if(note.start <= time && note.start+note.duration >= time){
			actives[note.note].enabled = true;
			actives[note.note].duration = float(note.duration);
			actives[note.note].start = float(note.start);
		}
	}
}

void MIDITrack::print() const {
	std::cout << "[INFO]: * Events (" << _events.size() << "): " << std::endl;
	for(auto& event : _events){
		event.print();
	}
	std::cout << "[INFO]: * Notes (" << _notes.size() << "): " << std::endl;
	for(auto& note : _notes){
		note.print();
	}
}

void MIDITrack::merge(MIDITrack & other){
	for(auto& note : other._notes){
		_notes.push_back(note);
	}
	std::sort(_notes.begin(), _notes.end(), [](const MIDINote & note1, const MIDINote & note2) { return(note1.start < note2.start); } );
}

std::pair<double, double> MIDITrack::computeNoteTimings(const std::vector<MIDITempo> & tempos, size_t start,size_t end, uint16_t upqn) const {
	double startTime;
	double endTime;

	size_t tid = 0;
	for(; tid < tempos.size(); ++tid){
		if((tid == tempos.size() - 1) || (tempos[tid+1].start > start)){
			// The note started before tid+1, we should use the previous one.
			startTime = tempos[tid].timestamp + computeUnitsDuration(tempos[tid].tempo, start - tempos[tid].start, upqn);
			break;
		}
	}
	// Start from the same one again.
	for(; tid < tempos.size(); ++tid){
		if((tid == tempos.size() - 1) || (tempos[tid+1].start > end)){
			// The note ended before tid+1, we should use the previous one.
			endTime = tempos[tid].timestamp + computeUnitsDuration(tempos[tid].tempo, end - tempos[tid].start, upqn);
			break;
		}
	}

	startTime /= 1000000.0;
	endTime /= 1000000.0;

	return std::make_pair(startTime, endTime);
}
