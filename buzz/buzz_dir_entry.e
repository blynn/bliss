class BUZZ_DIR_ENTRY
creation make
feature
    make(new_name : STRING; new_offset, new_size : INTEGER) is
    do
	!!name.copy(new_name)
	offset := new_offset
	size := new_size
    end

    name : STRING
    offset : INTEGER
    size : INTEGER
end
