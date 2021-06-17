function doGet(e) {
  Logger.log(JSON.stringify(e));  // view parameters
  var result = 'Ok'; // assume success
  if (e.parameter == undefined) {
    result = 'No Parameters';
  }
  else {
    var sheet_id = 'IDHere'; 		// Spreadsheet ID
    var spreadsheet = SpreadsheetApp.openById(sheet_id);
    var sheet;
    for (var param in e.parameter) {
      var value = stripQuotes(e.parameter[param]);
      var checkIn, userName;
      switch (param) {
        case 'Lembar':
          if (value == 1)
            checkIn = true;
          else if (value == 2)
            checkIn = false;
          break;
        case 'IDNum':
          if (value == [value1])
            userName = "A";
          else if (value == [value2])
            userName = "B";
          else if (value == [value3])
            userName = "C";
          else
            userName = "ETC";
          break;
      }
      if (userName == "A") {
        if (checkIn == true)
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("In A"));
        else
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("Out A"));
      }
      else if (userName == "B") {
        if (checkIn == true)
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("In B"));
        else
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("Out B"));
      }
      else if (userName == "C") {
        if (checkIn == true)
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("In C"));
        else
          sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("Out C"));
      }
      else
        sheet = SpreadsheetApp.setActiveSheet(spreadsheet.getSheetByName("ETC"));
    }
    var newRow = sheet.getLastRow() + 1;
    var rowData = [];
    rowData[0] = new Date();
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'IDNum': //Parameter
          rowData[1] = value;
          result = 'ID noted';
          break;
        case 'Lembar':
          break;
        default:
          result += "test failed";
      }
    }
    Logger.log(JSON.stringify(rowData));
    // Write new row below
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  sheet.getRange("A2:A").setNumberFormat("dd.MM.yyyy, HH:mm").setHorizontalAlignment("center");
  sheet.getRange("B2:B").setHorizontalAlignment("center");
  // Return result of operation
  return ContentService.createTextOutput(result);
}

/**
* Remove leading and trailing single or double quotes
*/
function stripQuotes(value) {
  return value.replace(/^["']|['"]$/g, "");
}
