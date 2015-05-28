def load_sc_data (file_path, num_neurons=0):
# load_sc_data Code to load up data which has been output from SpineCreator.
# Returns the time axis in milliseconds in the variable t.
#
# file_path is the path to the SpineCreator log file; it can be either
# the .bin file or the .xml file. If it's the .bin file, then
# num_neurons must be provided.

    import array as ar
    import numpy as np
    import struct

    # If file path ends with .bin, or .xml then find the root name.
    end = len(file_path)
    bin_end = file_path [end-4:end]
    xml_end = file_path [end-7:end]
    if (bin_end == '.bin'):
        base_path = file_path [:end-4]
    elif (xml_end == 'rep.xml'):
        base_path = file_path [:end-7]
    else:
        # Error
        print 'Error: Bad SpineCreator log file name: ' + file_path
        return (0,0,0)

    xml_file = base_path + 'rep.xml'
    bin_file = base_path + '.bin'

    # Timestep size. Initialise to 1
    dt = 1

    # Re-implement me!
    if (num_neurons == 0):
        # Find the number of neurons in the binary log file from the
        # xml file.
        import xml.etree.ElementTree as et
        tree = et.parse(xml_file)
        root = tree.getroot()

        # Assumed Analog Log here. May be wrong for event log.
        logFileType = root.find('.//LogFileType')
        if logFileType.text != 'binary':
            print 'File described by ', xml_file, ' is not marked as being in binary format.'
            return(0,0,0)

        # Log end is in steps of size dt. Unused at present even
        # though this is in the UI? Also may need logStartTime in
        # future to generate t.
        logEndTime = float(root.find('.//LogEndTime').text)
        num_neurons = int(root.find('.//LogAll').get('size'))
        # Timestep is specified in milliseconds
        dt = float(root.find('.//TimeStep').get('dt'))
        #print 'End time =', logEndTime, 'ms  num_neurons =', num_neurons, ' dt =', dt, 'ms'

    struct_fmt = 'd'
    struct_len = struct.calcsize(struct_fmt)
    struct_unpack = struct.Struct(struct_fmt).unpack_from

    data = []
    with open(bin_file, "rb") as fid:
        while True:
            dat = fid.read(struct_len)
            if not dat: break
            s = struct_unpack(dat)
            data.append(s[0])
    count = len(data)

    # Construct time series in milliseconds.
    t = np.linspace (0, (dt*count)-dt, count)

    # return data, count, t in a list. Could be a tuple?
    return (data, count, t)
